#include "mtp.h"
#include "../../fs/vfs.h"
#include "../../kernel/string.h"
#include "../../kernel/heap.h"

extern void print_serial(const char *);
extern void k_itoa(int n, char *s);
extern void k_itoa_hex(uint32_t n, char *s);
extern void kernel_heartbeat(void);
extern void kernel_poll_events(void);

// Rust xHCI FFI exports
extern int xhci_mtp_is_connected(void);
extern int xhci_mtp_write(const uint8_t *data, uint32_t len);
extern int xhci_mtp_read(uint8_t *buffer, uint32_t max_len);
extern int xhci_mtp_read_timeout(uint8_t *buffer, uint32_t max_len, uint32_t timeout_ms);
extern uint32_t rust_xhci_rescan(void);

static uint32_t g_mtp_tx_id = 1;
static int g_mtp_session_active = 0;
static uint32_t g_mtp_active_storage = 0;
static int g_mtp_quiet = 0;  // Suppress verbose serial logging during streaming
static int g_mtp_busy = 0;   // Mutex protecting MTP hardware & buffers against re-entrant calls

static uint8_t g_mtp_cmd_buf[64];
static uint8_t g_mtp_resp_buf[64];
static uint8_t g_mtp_data_buf[16384];

// Decode MTP UTF-16LE string into ASCII/UTF-8
static void mtp_decode_string(const uint8_t **ptr, char *out, int max_len) {
    if (!ptr || !*ptr || !out || max_len <= 0) return;
    uint8_t num_chars = **ptr;
    (*ptr)++;
    if (num_chars == 0) {
        out[0] = 0;
        return;
    }
    int out_idx = 0;
    for (int i = 0; i < num_chars && out_idx < max_len - 1; i++) {
        uint16_t ch = (*ptr)[0] | (((uint16_t)(*ptr)[1]) << 8);
        *ptr += 2;
        if (ch == 0) break;
        if (ch < 128) {
            out[out_idx++] = (char)ch;
        } else {
            out[out_idx++] = '_';
        }
    }
    out[out_idx] = 0;
}

static int mtp_send_cmd(uint16_t op, int num_params, uint32_t p1, uint32_t p2, uint32_t p3) {
    uint32_t len = sizeof(mtp_header_t) + num_params * sizeof(uint32_t);
    mtp_header_t *hdr = (mtp_header_t *)g_mtp_cmd_buf;
    hdr->length = len;
    hdr->container_type = MTP_CONTAINER_COMMAND;
    hdr->code = op;
    // MTP spec ISO 15740 mandates TransactionID = 0 for OpenSession
    if (op == MTP_OP_OPEN_SESSION) {
        hdr->transaction_id = 0;
        g_mtp_tx_id = 0;
    } else {
        hdr->transaction_id = ++g_mtp_tx_id;
    }

    uint32_t *params = (uint32_t *)(g_mtp_cmd_buf + sizeof(mtp_header_t));
    if (num_params >= 1) params[0] = p1;
    if (num_params >= 2) params[1] = p2;
    if (num_params >= 3) params[2] = p3;

    if (!g_mtp_quiet) {
        char dbg[32];
        print_serial("MTP send_cmd: op=0x");
        k_itoa_hex(op, dbg); print_serial(dbg);
        print_serial(" tx=");
        k_itoa((int)hdr->transaction_id, dbg); print_serial(dbg);
        print_serial(" len=");
        k_itoa((int)len, dbg); print_serial(dbg);
        print_serial("\n");
    }

    return xhci_mtp_write(g_mtp_cmd_buf, len);
}

static uint8_t g_mtp_buffered_resp[64];
static int g_mtp_buffered_resp_len = 0;

// Reliably read an entire MTP DATA container across multiple USB packets.
// If the device sends more data than max_len, drains the excess so the
// subsequent RESPONSE container is properly aligned.
static int mtp_read_full_data_timeout(uint8_t *buf, uint32_t max_len, uint32_t timeout_ms) {
    if (!buf || max_len < sizeof(mtp_header_t)) return -1;

    int r = xhci_mtp_read_timeout(buf, max_len, timeout_ms);
    if (r < (int)sizeof(mtp_header_t)) return r;

    mtp_header_t *hdr = (mtp_header_t *)buf;
    if (hdr->container_type != MTP_CONTAINER_DATA) {
        // Device sent an immediate response (e.g. error) instead of data container
        if (hdr->container_type == MTP_CONTAINER_RESPONSE) {
            int copy_len = (r < (int)sizeof(g_mtp_buffered_resp)) ? r : (int)sizeof(g_mtp_buffered_resp);
            memcpy(g_mtp_buffered_resp, buf, copy_len);
            g_mtp_buffered_resp_len = copy_len;
            if (!g_mtp_quiet) {
                print_serial("MTP: Immediate response container captured in read_full_data, code=0x");
                char dbg[16]; k_itoa_hex(hdr->code, dbg); print_serial(dbg); print_serial("\n");
            }
        }
        return 0; // 0 bytes of data payload
    }

    uint32_t total_expected = hdr->length;
    uint32_t to_read = (total_expected < max_len) ? total_expected : max_len;
    uint32_t total_received = (uint32_t)r;

    while (total_received < to_read) {
        uint32_t want = to_read - total_received;
        int chunk = xhci_mtp_read_timeout(buf + total_received, want, timeout_ms);
        if (chunk <= 0) break;
        total_received += chunk;
    }

    // If total_expected exceeds max_len, drain remaining bytes rapidly using 16KB chunks
    if (total_expected > total_received) {
        uint32_t excess = total_expected - total_received;
        static uint8_t drain[16384];
        while (excess > 0) {
            uint32_t want = (excess > sizeof(drain)) ? sizeof(drain) : excess;
            int c = xhci_mtp_read_timeout(drain, want, 2000);
            if (c <= 0) break;
            excess -= c;
        }
    }

    return (int)total_received;
}

static int mtp_read_full_data(uint8_t *buf, uint32_t max_len) {
    return mtp_read_full_data_timeout(buf, max_len, 5000);
}

static int mtp_read_response(uint16_t *resp_code) {
    if (g_mtp_buffered_resp_len >= (int)sizeof(mtp_header_t)) {
        memcpy(g_mtp_resp_buf, g_mtp_buffered_resp, g_mtp_buffered_resp_len);
        g_mtp_buffered_resp_len = 0;
        mtp_header_t *hdr = (mtp_header_t *)g_mtp_resp_buf;
        if (resp_code) *resp_code = hdr->code;
        return (hdr->code == MTP_RESP_OK || hdr->code == MTP_RESP_SESSION_ALREADY_OPEN) ? 0 : -3;
    }

    for (int retry = 0; retry < 4; retry++) {
        kernel_heartbeat();
        int r = xhci_mtp_read_timeout(g_mtp_resp_buf, sizeof(g_mtp_resp_buf), 1000);
        if (r == 0) {
            if (!g_mtp_quiet) print_serial("MTP resp: ZLP, reading next\n");
            continue;
        }
        if (r < (int)sizeof(mtp_header_t)) {
            if (!g_mtp_quiet) print_serial("MTP resp: too short\n");
            if (retry + 1 < 4) continue;
            return -1;
        }
        mtp_header_t *hdr = (mtp_header_t *)g_mtp_resp_buf;

        if (!g_mtp_quiet) {
            char dbg[32];
            print_serial("MTP resp: code=0x");
            k_itoa_hex(hdr->code, dbg); print_serial(dbg);
            print_serial(" type=0x");
            k_itoa_hex(hdr->container_type, dbg); print_serial(dbg);
            print_serial("\n");
        }

        if (hdr->container_type == MTP_CONTAINER_RESPONSE) {
            if (resp_code) *resp_code = hdr->code;
            return (hdr->code == MTP_RESP_OK || hdr->code == MTP_RESP_SESSION_ALREADY_OPEN) ? 0 : -3;
        }

        // If container_type != RESPONSE, scan buffer to check if a response header starts at an offset
        if (r >= (int)sizeof(mtp_header_t) + 4) {
            for (int off = 1; off <= r - (int)sizeof(mtp_header_t); off++) {
                mtp_header_t *cand = (mtp_header_t *)(g_mtp_resp_buf + off);
                if (cand->container_type == MTP_CONTAINER_RESPONSE) {
                    if (!g_mtp_quiet) {
                        print_serial("MTP: Found aligned response header at offset ");
                        char ob[16]; k_itoa(off, ob); print_serial(ob); print_serial("\n");
                    }
                    if (resp_code) *resp_code = cand->code;
                    return (cand->code == MTP_RESP_OK || cand->code == MTP_RESP_SESSION_ALREADY_OPEN) ? 0 : -3;
                }
            }
        }

        // If orphaned data packet arrived, drain any leftover rapidly and retry
        if (hdr->container_type == MTP_CONTAINER_DATA) {
            if (hdr->length > (uint32_t)r) {
                uint32_t excess = hdr->length - (uint32_t)r;
                static uint8_t drain[16384];
                while (excess > 0) {
                    uint32_t want = (excess > sizeof(drain)) ? sizeof(drain) : excess;
                    int c = xhci_mtp_read_timeout(drain, want, 500);
                    if (c <= 0) break;
                    excess -= c;
                }
            }
        }
    }
    return -2;
}

extern uint64_t get_timer_ms_hires(void);

int mtp_is_connected(void) {
    return xhci_mtp_is_connected();
}

int mtp_rescan_if_needed(void) {
    if (xhci_mtp_is_connected()) return 1;
    static uint64_t last_scan_ms = 0;
    uint64_t now = get_timer_ms_hires();
    if (last_scan_ms != 0 && (now - last_scan_ms < 4000)) {
        return 0; // Throttle to at most once per 4 seconds
    }
    last_scan_ms = now;
    kernel_heartbeat();
    print_serial("MTP: Rescanning for USB devices...\n");
    rust_xhci_rescan();
    return xhci_mtp_is_connected();
}

void mtp_poll_hotplug(void) {
    static uint64_t last_poll_ms = 0;
    uint64_t now = get_timer_ms_hires();
    if (last_poll_ms != 0 && (now - last_poll_ms < 350)) {
        return; // Poll every 350ms
    }
    last_poll_ms = now;

    extern int rust_xhci_poll_hotplug(void);
    int res = rust_xhci_poll_hotplug();
    if (res == 1) {
        print_serial("MTP HOTPLUG: Phone connected while OS is running!\n");
        extern void notification_show(const char *title, const char *subtitle, const char *action_hint, int action_type);
        notification_show("Phone Connected", "Galaxy M35 (MTP Media Transfer)", "Click to open in Explorer ->", 1);

        extern void explorer_on_phone_hotplug(int connected);
        explorer_on_phone_hotplug(1);
    } else if (res == -1) {
        print_serial("MTP HOTPLUG: Phone disconnected!\n");
        g_mtp_session_active = 0;
        extern void notification_show(const char *title, const char *subtitle, const char *action_hint, int action_type);
        notification_show("Phone Disconnected", "USB Device Unplugged", "", 0);

        extern void explorer_on_phone_hotplug(int connected);
        explorer_on_phone_hotplug(0);
    }
}


int mtp_open_session(void) {
    if (!xhci_mtp_is_connected()) return -1;
    if (g_mtp_session_active) return 0;

    kernel_heartbeat();

    print_serial("MTP: Opening session with mobile device...\n");
    mtp_send_cmd(MTP_OP_OPEN_SESSION, 1, 1, 0, 0);

    uint16_t code = 0;
    if (mtp_read_response(&code) == 0) {
        g_mtp_session_active = 1;
        print_serial("MTP: Session OPEN SUCCESS!\n");
        return 0;
    }

    // If initial open failed, try closing and reopening
    print_serial("MTP: Session open attempt 1 failed, trying close & re-open...\n");
    mtp_send_cmd(MTP_OP_CLOSE_SESSION, 0, 0, 0, 0);
    mtp_read_response(NULL);

    mtp_send_cmd(MTP_OP_OPEN_SESSION, 1, 1, 0, 0);
    if (mtp_read_response(&code) == 0) {
        g_mtp_session_active = 1;
        print_serial("MTP: Session OPEN SUCCESS (after close)!\n");
        return 0;
    }

    print_serial("MTP: Session open failed.\n");
    return -1;
}

int mtp_close_session(void) {
    if (!g_mtp_session_active) return 0;
    mtp_send_cmd(MTP_OP_CLOSE_SESSION, 0, 0, 0, 0);
    mtp_read_response(NULL);
    g_mtp_session_active = 0;
    g_mtp_active_storage = 0;
    return 0;
}

int mtp_get_storage_id(uint32_t *storage_id, char *storage_desc, int max_desc) {
    if (!g_mtp_session_active && mtp_open_session() != 0) return -1;

    mtp_send_cmd(MTP_OP_GET_STORAGE_IDS, 0, 0, 0, 0);
    int r = mtp_read_full_data(g_mtp_data_buf, sizeof(g_mtp_data_buf));
    if (r < (int)(sizeof(mtp_header_t) + 4)) {
        mtp_read_response(NULL);
        return -1;
    }

    print_serial("MTP: StorageIDs raw (bytes=");
    char dbg[16];
    k_itoa(r, dbg);
    print_serial(dbg);
    print_serial("): ");
    for (int i = 0; i < (r < 32 ? r : 32); i++) {
        char h[4];
        k_itoa_hex(g_mtp_data_buf[i], h);
        print_serial(h);
        print_serial(" ");
    }
    print_serial("\n");

    uint32_t count = *(uint32_t *)(g_mtp_data_buf + sizeof(mtp_header_t));
    print_serial("MTP: Storage count=");
    k_itoa(count, dbg);
    print_serial(dbg);
    print_serial("\n");

    mtp_read_response(NULL);

    if (count == 0) {
        print_serial("MTP: Phone has 0 storages (Phone is locked or USB permission pending)\n");
        g_mtp_active_storage = 0;
        mtp_close_session();
        return -2;
    }

    uint32_t *ids = (uint32_t *)(g_mtp_data_buf + sizeof(mtp_header_t) + 4);
    uint32_t primary_id = ids[0];

    for (uint32_t i = 0; i < count && i < 8; i++) {
        print_serial("MTP: Storage[");
        k_itoa(i, dbg);
        print_serial(dbg);
        print_serial("]=0x");
        k_itoa_hex(ids[i], dbg);
        print_serial(dbg);
        print_serial("\n");
    }

    g_mtp_active_storage = primary_id;
    if (storage_id) *storage_id = primary_id;

    // Fetch storage info
    mtp_send_cmd(MTP_OP_GET_STORAGE_INFO, 1, primary_id, 0, 0);
    r = mtp_read_full_data(g_mtp_data_buf, sizeof(g_mtp_data_buf));
    if (r > (int)sizeof(mtp_header_t) + 26) {
        const uint8_t *ptr = g_mtp_data_buf + sizeof(mtp_header_t) + 26;
        if (storage_desc && max_desc > 0) {
            mtp_decode_string(&ptr, storage_desc, max_desc);
            if (storage_desc[0] == 0) {
                strcpy(storage_desc, "Internal Shared Storage");
            }
        }
    } else {
        if (storage_desc && max_desc > 0) {
            strcpy(storage_desc, "Internal Shared Storage");
        }
    }
    mtp_read_response(NULL);
    return 0;
}

int mtp_fetch_handles(uint32_t parent_handle, uint32_t **out_handles, int *out_count, uint32_t *out_st_id) {
    if (g_mtp_busy) return -16;
    g_mtp_busy = 1;

    if (!g_mtp_session_active && mtp_open_session() != 0) {
        g_mtp_busy = 0;
        return -1;
    }
    if (g_mtp_active_storage == 0) {
        int s = mtp_get_storage_id(NULL, NULL, 0);
        if (s != 0) {
            print_serial("MTP: Storage not available, status=");
            char dbg[16]; k_itoa(s, dbg); print_serial(dbg); print_serial("\n");
            g_mtp_busy = 0;
            return s;
        }
    }

    uint32_t st_id = (g_mtp_active_storage != 0) ? g_mtp_active_storage : 0xFFFFFFFF;
    // In Android MTP, 0xFFFFFFFF means root objects (no parent).
    // Passing 0 on Android causes it to return all 22,000+ files across the entire phone!
    uint32_t p_handle = (parent_handle == 0) ? 0xFFFFFFFF : parent_handle;

    char dbg[16];
    print_serial("MTP: GetObjectHandles storage=0x");
    k_itoa_hex(st_id, dbg);
    print_serial(dbg);
    print_serial(" parent=0x");
    k_itoa_hex(p_handle, dbg);
    print_serial(dbg);
    print_serial("\n");

    mtp_send_cmd(MTP_OP_GET_OBJECT_HANDLES, 3, st_id, 0, p_handle);
    int r = mtp_read_full_data_timeout(g_mtp_data_buf, sizeof(g_mtp_data_buf), 20000);
    uint32_t count = 0;
    if (r >= (int)(sizeof(mtp_header_t) + 4)) {
        count = *(uint32_t *)(g_mtp_data_buf + sizeof(mtp_header_t));
    }
    mtp_read_response(NULL);

    // Fallback: If 0xFFFFFFFF returned 0 on root, try 0x0 for legacy devices
    if (count == 0 && parent_handle == 0) {
        print_serial("MTP: Trying fallback root parent=0x0...\n");
        mtp_send_cmd(MTP_OP_GET_OBJECT_HANDLES, 3, st_id, 0, 0);
        r = mtp_read_full_data_timeout(g_mtp_data_buf, sizeof(g_mtp_data_buf), 20000);
        if (r >= (int)(sizeof(mtp_header_t) + 4)) {
            count = *(uint32_t *)(g_mtp_data_buf + sizeof(mtp_header_t));
        }
        mtp_read_response(NULL);
    }

    // Fallback: If subfolder query returned 0, try with wildcard storage 0xFFFFFFFF
    if (count == 0 && parent_handle != 0 && st_id != 0xFFFFFFFF) {
        print_serial("MTP: Subfolder returned 0 with st_id, trying storage 0xFFFFFFFF...\n");
        mtp_send_cmd(MTP_OP_GET_OBJECT_HANDLES, 3, 0xFFFFFFFF, 0, p_handle);
        r = mtp_read_full_data_timeout(g_mtp_data_buf, sizeof(g_mtp_data_buf), 20000);
        if (r >= (int)(sizeof(mtp_header_t) + 4)) {
            count = *(uint32_t *)(g_mtp_data_buf + sizeof(mtp_header_t));
        }
        mtp_read_response(NULL);
    }

    uint32_t *handles = (uint32_t *)(g_mtp_data_buf + sizeof(mtp_header_t) + 4);

    print_serial("MTP: Handles count=");
    k_itoa(count, dbg);
    print_serial(dbg);
    print_serial("\n");

    if (out_st_id) *out_st_id = st_id;

    if (count == 0) {
        if (out_handles) *out_handles = NULL;
        if (out_count) *out_count = 0;
        g_mtp_busy = 0;
        return 0;
    }

    uint32_t *local_handles = (uint32_t *)kmalloc(count * sizeof(uint32_t));
    if (!local_handles) {
        if (out_handles) *out_handles = NULL;
        if (out_count) *out_count = 0;
        g_mtp_busy = 0;
        return 0;
    }
    for (uint32_t i = 0; i < count; i++) {
        local_handles[i] = handles[i];
    }

    if (out_handles) *out_handles = local_handles;
    if (out_count) *out_count = (int)count;

    g_mtp_busy = 0;
    return 0;
}

int mtp_fetch_single_object_info(uint32_t handle, uint32_t storage_id, mtp_object_entry_t *entry) {
    if (!entry) return -1;
    if (g_mtp_busy) return -16;
    g_mtp_busy = 1;

    entry->handle = handle;
    entry->storage_id = storage_id;
    entry->is_dir = 0;
    entry->size_bytes = 0;
    entry->format = 0;
    entry->name[0] = 0;

    g_mtp_quiet = 1;
    mtp_send_cmd(MTP_OP_GET_OBJECT_INFO, 1, handle, 0, 0);
    int info_len = mtp_read_full_data_timeout(g_mtp_data_buf, sizeof(g_mtp_data_buf), 500);
    if (info_len > (int)(sizeof(mtp_header_t) + 12)) {
        uint8_t *obj = g_mtp_data_buf + sizeof(mtp_header_t);
        entry->format = *(uint16_t *)(obj + 4);
        entry->size_bytes = *(uint32_t *)(obj + 8);
        entry->is_dir = (entry->format == MTP_FORMAT_ASSOCIATION);

        if (info_len >= (int)(sizeof(mtp_header_t) + 53)) {
            const uint8_t *str_ptr = obj + 52;
            mtp_decode_string(&str_ptr, entry->name, sizeof(entry->name));
        }
    }
    if (entry->name[0] == 0) {
        char b[16];
        k_itoa((int)handle, b);
        strcpy(entry->name, "item_");
        strcat(entry->name, b);
    }
    mtp_read_response(NULL);
    g_mtp_quiet = 0;

    g_mtp_busy = 0;
    return 0;
}

int mtp_list_directory_streaming(uint32_t parent_handle, mtp_object_entry_t *entries, int max_entries,
                                 mtp_entry_callback_t cb, void *user_data) {
    uint32_t *handles = NULL;
    int count = 0;
    uint32_t st_id = 0;
    int res = mtp_fetch_handles(parent_handle, &handles, &count, &st_id);
    if (res != 0) return res;
    if (count == 0 || !handles) return 0;

    int n = (count < max_entries) ? count : max_entries;
    for (int i = 0; i < n; i++) {
        kernel_heartbeat();
        kernel_poll_events();
        mtp_fetch_single_object_info(handles[i], st_id, &entries[i]);
        if (cb) {
            if (cb(&entries[i], i, n, user_data) < 0) {
                kfree(handles);
                return i + 1;
            }
        }
    }
    kfree(handles);
    return n;
}

int mtp_list_directory(uint32_t parent_handle, mtp_object_entry_t *entries, int max_entries) {
    return mtp_list_directory_streaming(parent_handle, entries, max_entries, NULL, NULL);
}

int mtp_pull_file(uint32_t object_handle, const char *local_dest_path) {
    if (g_mtp_busy) return -16;
    g_mtp_busy = 1;

    if (!g_mtp_session_active && mtp_open_session() != 0) {
        g_mtp_busy = 0;
        return -1;
    }

    char auto_name[128] = {0};
    uint32_t obj_size = 0;
    mtp_send_cmd(MTP_OP_GET_OBJECT_INFO, 1, object_handle, 0, 0);
    int r = mtp_read_full_data_timeout(g_mtp_data_buf, sizeof(g_mtp_data_buf), 3000);
    if (r > (int)(sizeof(mtp_header_t) + 12)) {
        uint8_t *obj = g_mtp_data_buf + sizeof(mtp_header_t);
        obj_size = *(uint32_t *)(obj + 8);
        if (r >= (int)(sizeof(mtp_header_t) + 53)) {
            const uint8_t *str_ptr = obj + 52;
            mtp_decode_string(&str_ptr, auto_name, sizeof(auto_name));
        }
    }
    mtp_read_response(NULL);

    char clean_dest[160];
    const char *out_path = (local_dest_path && local_dest_path[0]) ? local_dest_path : auto_name;
    if (!out_path[0]) out_path = "phone_download.bin";

    // If writing to /ram/, ensure the filename component does not exceed 31 chars
    if (strncmp(out_path, "/ram/", 5) == 0) {
        const char *raw_file = out_path + 5;
        char safe[32];
        extern void make_safe_ram_name(const char *orig_name, char *safe_out, int max_len);
        make_safe_ram_name(raw_file, safe, 31);
        strcpy(clean_dest, "/ram/");
        strcat(clean_dest, safe);
        out_path = clean_dest;
    }

    print_serial("MTP: Pulling handle 0x");
    char hstr[16];
    k_itoa_hex(object_handle, hstr);
    print_serial(hstr);
    print_serial(" to ");
    print_serial(out_path);
    print_serial("\n");

    int fd = vfs_open(out_path, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        print_serial("MTP: Failed to create local destination file: ");
        print_serial(out_path);
        print_serial("\n");
        g_mtp_busy = 0;
        return -2;
    }

    mtp_send_cmd(MTP_OP_GET_OBJECT, 1, object_handle, 0, 0);

    int total_received = 0;
    r = xhci_mtp_read_timeout(g_mtp_data_buf, sizeof(g_mtp_data_buf), 10000);
    if (r < (int)sizeof(mtp_header_t)) {
        print_serial("MTP: Failed initial read on GET_OBJECT\n");
        vfs_close(fd);
        vfs_unlink(out_path);
        mtp_read_response(NULL);
        g_mtp_busy = 0;
        return -3;
    }

    mtp_header_t *hdr = (mtp_header_t *)g_mtp_data_buf;
    if (hdr->container_type != MTP_CONTAINER_DATA) {
        print_serial("MTP: Unexpected container type on GET_OBJECT: 0x");
        char hex[16];
        k_itoa_hex(hdr->code, hex);
        print_serial(hex);
        print_serial("\n");
        vfs_close(fd);
        vfs_unlink(out_path);
        mtp_read_response(NULL);
        g_mtp_busy = 0;
        return -4;
    }

    uint32_t payload_len = (uint32_t)r - sizeof(mtp_header_t);
    if (payload_len > 0) {
        vfs_write(fd, g_mtp_data_buf + sizeof(mtp_header_t), payload_len);
        total_received += (int)payload_len;
    }

    uint32_t total_payload = 0;
    if (hdr->length != 0xFFFFFFFF && hdr->length >= sizeof(mtp_header_t)) {
        total_payload = hdr->length - sizeof(mtp_header_t);
    } else if (obj_size > 0) {
        total_payload = obj_size;
    }

    while (total_payload > 0 && total_received < (int)total_payload) {
        kernel_heartbeat();
        kernel_poll_events();
        uint32_t want = total_payload - (uint32_t)total_received;
        if (want > sizeof(g_mtp_data_buf)) want = sizeof(g_mtp_data_buf);
        r = xhci_mtp_read_timeout(g_mtp_data_buf, want, 10000);
        if (r <= 0) break;
        vfs_write(fd, g_mtp_data_buf, (uint32_t)r);
        total_received += r;
    }

    vfs_close(fd);
    mtp_read_response(NULL);

    print_serial("MTP: Pull complete, received ");
    char bstr[16];
    k_itoa(total_received, bstr);
    print_serial(bstr);
    print_serial(" bytes\n");

    g_mtp_busy = 0;
    return total_received;
}

int mtp_push_file(const char *local_src_path, uint32_t parent_handle, const char *phone_filename) {
    if (!g_mtp_session_active && mtp_open_session() != 0) return -1;
    if (g_mtp_active_storage == 0 && mtp_get_storage_id(NULL, NULL, 0) != 0) return -1;

    extern int vfs_open(const char *path, int flags);
    int fd = vfs_open(local_src_path, 0); // 0 = O_RDONLY
    if (fd < 0) {
        print_serial("MTP push: vfs_open failed for ");
        print_serial(local_src_path);
        print_serial("\n");
        return -2;
    }

    vfs_stat_t st;
    if (vfs_stat(local_src_path, &st) != 0) {
        vfs_close(fd);
        return -3;
    }
    uint32_t fsize = st.st_size;
    if (fsize == 0) fsize = st.size;
    uint32_t p_handle = parent_handle; // 0 for root

    const char *fn = phone_filename ? phone_filename : local_src_path;
    const char *last_s = fn;
    for (const char *p = fn; *p; p++) {
        if (*p == '/' || *p == '\\') last_s = p + 1;
    }
    fn = last_s;
    if (!fn[0]) fn = "upload.bin";

    print_serial("MTP: Pushing file '"); print_serial(fn);
    print_serial("' (size=");
    char dbg[16]; k_itoa(fsize, dbg); print_serial(dbg);
    print_serial(") to parent=0x");
    k_itoa_hex(p_handle, dbg); print_serial(dbg);
    print_serial("\n");

    memset(g_mtp_data_buf, 0, 512);
    mtp_header_t *hdr = (mtp_header_t *)g_mtp_data_buf;
    uint8_t *obj = g_mtp_data_buf + sizeof(mtp_header_t);

    *(uint32_t *)(obj + 0) = g_mtp_active_storage;
    *(uint16_t *)(obj + 4) = MTP_FORMAT_UNDEFINED;
    *(uint32_t *)(obj + 8) = fsize;
    *(uint32_t *)(obj + 38) = p_handle;

    int fn_len = strlen(fn);
    uint8_t *fn_ptr = obj + 52;
    *fn_ptr++ = fn_len + 1;
    for (int i = 0; i < fn_len; i++) {
        *fn_ptr++ = (uint8_t)fn[i];
        *fn_ptr++ = 0;
    }
    *fn_ptr++ = 0;
    *fn_ptr++ = 0;

    uint32_t dataset_len = (uint32_t)(fn_ptr - obj);
    hdr->length = sizeof(mtp_header_t) + dataset_len;
    hdr->container_type = MTP_CONTAINER_DATA;
    hdr->code = MTP_OP_SEND_OBJECT_INFO;

    mtp_send_cmd(MTP_OP_SEND_OBJECT_INFO, 2, g_mtp_active_storage, p_handle, 0);
    hdr->transaction_id = g_mtp_tx_id;
    xhci_mtp_write(g_mtp_data_buf, hdr->length);

    uint16_t code = 0;
    if (mtp_read_response(&code) != 0) {
        print_serial("MTP: SendObjectInfo rejected, code=0x");
        k_itoa_hex(code, dbg); print_serial(dbg); print_serial("\n");
        vfs_close(fd);
        return -4;
    }

    mtp_send_cmd(MTP_OP_SEND_OBJECT, 0, 0, 0, 0);

    hdr->length = sizeof(mtp_header_t) + fsize;
    hdr->container_type = MTP_CONTAINER_DATA;
    hdr->code = MTP_OP_SEND_OBJECT;
    hdr->transaction_id = g_mtp_tx_id;

    int read_bytes = vfs_read(fd, g_mtp_data_buf + sizeof(mtp_header_t), sizeof(g_mtp_data_buf) - sizeof(mtp_header_t));
    if (read_bytes < 0) read_bytes = 0;
    if ((uint32_t)read_bytes > fsize) read_bytes = fsize;

    xhci_mtp_write(g_mtp_data_buf, sizeof(mtp_header_t) + read_bytes);
    int remaining = fsize - read_bytes;
    while (remaining > 0) {
        uint32_t to_read = (remaining > (int)sizeof(g_mtp_data_buf)) ? sizeof(g_mtp_data_buf) : (uint32_t)remaining;
        int chunk = vfs_read(fd, g_mtp_data_buf, to_read);
        if (chunk <= 0) break;
        xhci_mtp_write(g_mtp_data_buf, chunk);
        remaining -= chunk;
    }
    vfs_close(fd);
    mtp_read_response(NULL);
    print_serial("MTP: Push file complete!\n");
    return (int)fsize;
}

int mtp_delete_object(uint32_t object_handle) {
    if (!object_handle) return -1;
    if (!g_mtp_session_active && mtp_open_session() != 0) return -1;

    print_serial("MTP: DeleteObject handle=0x");
    char dbg[16]; k_itoa_hex(object_handle, dbg); print_serial(dbg); print_serial("\n");

    mtp_send_cmd(MTP_OP_DELETE_OBJECT, 2, object_handle, 0, 0);
    uint16_t resp = 0;
    int r = mtp_read_response(&resp);
    return (r == 0 && resp == MTP_RESP_OK) ? 0 : -1;
}

int mtp_move_object(uint32_t object_handle, uint32_t target_parent_handle) {
    if (!object_handle) return -1;
    if (!g_mtp_session_active && mtp_open_session() != 0) return -1;
    if (g_mtp_active_storage == 0 && mtp_get_storage_id(NULL, NULL, 0) != 0) return -1;

    print_serial("MTP: MoveObject handle=0x");
    char dbg[16]; k_itoa_hex(object_handle, dbg); print_serial(dbg);
    print_serial(" to parent=0x");
    k_itoa_hex(target_parent_handle, dbg); print_serial(dbg);
    print_serial("\n");

    mtp_send_cmd(MTP_OP_MOVE_OBJECT, 3, object_handle, g_mtp_active_storage, target_parent_handle);
    uint16_t resp = 0;
    int r = mtp_read_response(&resp);
    if (r == 0 && resp == MTP_RESP_OK) {
        print_serial("MTP: MoveObject SUCCESS!\n");
        return 0;
    }
    print_serial("MTP: MoveObject failed, resp=0x");
    k_itoa_hex(resp, dbg); print_serial(dbg); print_serial("\n");
    return -1;
}

int mtp_copy_object(uint32_t object_handle, uint32_t target_parent_handle) {
    if (!object_handle) return -1;
    if (!g_mtp_session_active && mtp_open_session() != 0) return -1;
    if (g_mtp_active_storage == 0 && mtp_get_storage_id(NULL, NULL, 0) != 0) return -1;

    print_serial("MTP: CopyObject handle=0x");
    char dbg[16]; k_itoa_hex(object_handle, dbg); print_serial(dbg);
    print_serial(" to parent=0x");
    k_itoa_hex(target_parent_handle, dbg); print_serial(dbg);
    print_serial("\n");

    mtp_send_cmd(MTP_OP_COPY_OBJECT, 3, object_handle, g_mtp_active_storage, target_parent_handle);
    uint16_t resp = 0;
    int r = mtp_read_response(&resp);
    if (r == 0 && resp == MTP_RESP_OK) {
        print_serial("MTP: CopyObject SUCCESS!\n");
        return 0;
    }
    return -1;
}
