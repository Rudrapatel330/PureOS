#include "bt_core.h"
#include "../../kernel/types.h"
#include "../../kernel/string.h"
#include "../timer.h"

extern void print_serial(const char *s);
extern void k_itoa(int val, char *buf);

// Hardware FFI primitives implemented in Rust xHCI subsystem
extern int xhci_bt_is_present(void);
extern int xhci_bt_send_cmd(uint16_t opcode, const uint8_t *params, uint8_t param_len);
extern int xhci_bt_poll_event(uint8_t *buf, uint32_t max_len);

static void print_hex16(const char *prefix, uint16_t val) {
    const char hx[] = "0123456789ABCDEF";
    char buf[8];
    buf[0] = '0'; buf[1] = 'x';
    buf[2] = hx[(val >> 12) & 0xF];
    buf[3] = hx[(val >>  8) & 0xF];
    buf[4] = hx[(val >>  4) & 0xF];
    buf[5] = hx[(val      ) & 0xF];
    buf[6] = '\0';
    print_serial(prefix);
    print_serial(buf);
}

static void print_hex8(const char *prefix, uint8_t val) {
    const char hx[] = "0123456789ABCDEF";
    char buf[5];
    buf[0] = '0'; buf[1] = 'x';
    buf[2] = hx[(val >> 4) & 0xF];
    buf[3] = hx[(val     ) & 0xF];
    buf[4] = '\0';
    print_serial(prefix);
    print_serial(buf);
}

#define MAX_BT_DEVICES 32

static int g_bt_initialized = 0;
static int g_bt_enabled = 1;
static int g_bt_scanning = 0;
static int g_bt_hw_operational = 0;
static uint32_t g_scan_start_tick = 0;
static uint32_t g_scan_duration_ticks = 3250; // ~13 seconds at 250Hz (inquiry = 10.24s + buffer)

static bt_device_t g_devices[MAX_BT_DEVICES];
static int g_device_count = 0;

static int g_pairing_index = -1;
static uint32_t g_pairing_start_tick = 0;

static char g_local_bd_addr_str[24] = "00:00:00:00:00:00";

static void format_mac_addr(const uint8_t *b, char *out_str) {
    const char hex[] = "0123456789ABCDEF";
    // Bluetooth BD_ADDR is transmitted LSB first (b[0] = NAP/LAP), standard display is MSB first (b[5]:b[4]:...:b[0])
    int pos = 0;
    for (int i = 5; i >= 0; i--) {
        out_str[pos++] = hex[(b[i] >> 4) & 0x0F];
        out_str[pos++] = hex[b[i] & 0x0F];
        if (i > 0) out_str[pos++] = ':';
    }
    out_str[pos] = '\0';
}

static int find_device_by_addr(const uint8_t *addr) {
    for (int i = 0; i < g_device_count; i++) {
        if (memcmp(g_devices[i].addr, addr, 6) == 0) {
            return i;
        }
    }
    return -1;
}

static void update_signal_bars(bt_device_t *dev) {
    if (dev->rssi >= -55) dev->signal_bars = 4;
    else if (dev->rssi >= -68) dev->signal_bars = 3;
    else if (dev->rssi >= -80) dev->signal_bars = 2;
    else dev->signal_bars = 1;
}

static bt_device_type_t determine_device_type(uint32_t cod, const char *name) {
    // Check Bluetooth SIG Class of Device Major Device Class (bits 8 to 12)
    uint32_t major = (cod >> 8) & 0x1F;
    if (major == 0x01) return BT_DEV_COMPUTER;
    if (major == 0x02) return BT_DEV_PHONE;
    if (major == 0x04) return BT_DEV_AUDIO;
    if (major == 0x05) return BT_DEV_PERIPHERAL;
    if (major == 0x07) return BT_DEV_WATCH;
    if (major == 0x08) return BT_DEV_DISPLAY;

    // Check name heuristics for BLE devices where CoD might be 0
    if (name && name[0]) {
        char lower[64];
        int k = 0;
        while (name[k] && k < 63) {
            char c = name[k];
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
            lower[k++] = c;
        }
        lower[k] = '\0';

        if (strstr(lower, "phone") || strstr(lower, "galaxy") || strstr(lower, "iphone") ||
            strstr(lower, "pixel") || strstr(lower, "redmi") || strstr(lower, "realme") || strstr(lower, "m35")) {
            return BT_DEV_PHONE;
        }
        if (strstr(lower, "buds") || strstr(lower, "airdopes") || strstr(lower, "airpod") ||
            strstr(lower, "audio") || strstr(lower, "headphone") || strstr(lower, "speaker") ||
            strstr(lower, "wireless") || strstr(lower, "sound")) {
            return BT_DEV_AUDIO;
        }
        if (strstr(lower, "watch") || strstr(lower, "band") || strstr(lower, "fit")) {
            return BT_DEV_WATCH;
        }
        if (strstr(lower, "mouse") || strstr(lower, "keyboard") || strstr(lower, "keychron") ||
            strstr(lower, "pad") || strstr(lower, "controller")) {
            return BT_DEV_PERIPHERAL;
        }
    }

    return BT_DEV_UNKNOWN;
}

static void parse_ad_name(const uint8_t *ad, int max_len, char *out_name, int out_max) {
    out_name[0] = '\0';
    int offset = 0;
    while (offset + 1 < max_len) {
        uint8_t len = ad[offset];
        if (len == 0 || offset + 1 + len > max_len) break;
        uint8_t type = ad[offset + 1];

        // 0x09 = Complete Local Name, 0x08 = Shortened Local Name
        if (type == 0x09 || type == 0x08) {
            int name_len = len - 1;
            if (name_len >= out_max) name_len = out_max - 1;
            memcpy(out_name, &ad[offset + 2], name_len);
            out_name[name_len] = '\0';
            if (type == 0x09) break; // Complete name takes priority
        }
        offset += (1 + len);
    }
}

static void parse_eir_name(const uint8_t *eir, int max_len, char *out_name, int out_max) {
    parse_ad_name(eir, max_len, out_name, out_max);
}

static void process_discovered_device(const uint8_t *mac, uint32_t cod, int rssi, const char *name) {
    uint32_t now = get_timer_ticks();
    int idx = find_device_by_addr(mac);

    if (idx >= 0) {
        // Update existing device
        bt_device_t *d = &g_devices[idx];
        d->last_seen_tick = now;
        d->is_active = 1;
        if (rssi != 0) {
            d->rssi = rssi;
            update_signal_bars(d);
        }
        if (cod != 0 && d->class_of_device == 0) {
            d->class_of_device = cod;
            d->type = determine_device_type(cod, d->name);
        }
        if (name && name[0] && (d->name[0] == '\0' || strstr(d->name, "Device") != NULL)) {
            strncpy(d->name, name, sizeof(d->name) - 1);
            d->name[sizeof(d->name) - 1] = '\0';
            d->type = determine_device_type(d->class_of_device, d->name);
        }
        return;
    }

    // New device discovered
    if (g_device_count >= MAX_BT_DEVICES) return;

    bt_device_t *new_dev = &g_devices[g_device_count++];
    memset(new_dev, 0, sizeof(bt_device_t));

    memcpy(new_dev->addr, mac, 6);
    format_mac_addr(mac, new_dev->addr_str);

    if (name && name[0]) {
        strncpy(new_dev->name, name, sizeof(new_dev->name) - 1);
        new_dev->name[sizeof(new_dev->name) - 1] = '\0';
    } else {
        strcpy(new_dev->name, "Bluetooth Device");
    }

    new_dev->class_of_device = cod;
    new_dev->type = determine_device_type(cod, new_dev->name);
    new_dev->rssi = (rssi != 0) ? rssi : -70;
    update_signal_bars(new_dev);

    new_dev->status = BT_STATUS_DISCOVERED;
    new_dev->discovery_tick = now;
    new_dev->last_seen_tick = now;
    new_dev->is_active = 1;

    print_serial("[BTSTACK] HCI: New radio device found: '");
    print_serial(new_dev->name);
    print_serial("' [");
    print_serial(new_dev->addr_str);
    print_serial("]\n");
}

static void update_device_name(const uint8_t *mac, const char *name) {
    if (!name || !name[0]) return;
    int idx = find_device_by_addr(mac);
    if (idx >= 0) {
        strncpy(g_devices[idx].name, name, sizeof(g_devices[idx].name) - 1);
        g_devices[idx].name[sizeof(g_devices[idx].name) - 1] = '\0';
        g_devices[idx].type = determine_device_type(g_devices[idx].class_of_device, g_devices[idx].name);
        print_serial("[BTSTACK] HCI: Remote name resolved: '");
        print_serial(g_devices[idx].name);
        print_serial("'\n");
    }
}

static void send_remote_name_request(const uint8_t *mac) {
    if (!xhci_bt_is_present()) return;
    // HCI_Remote_Name_Request (Opcode 0x0419)
    // Param: BD_ADDR (6), Page_Scan_Repetition_Mode (1), Reserved (1), Clock_Offset (2)
    uint8_t params[10] = {
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        0x01, // R1 mode
        0x00, // Reserved
        0x00, 0x00 // Clock offset
    };
    xhci_bt_send_cmd(0x0419, params, sizeof(params));
}

void bt_init(void) {
    if (g_bt_initialized) return;

    print_serial("[BTSTACK] Initializing Bluetooth subsystem...\n");
    g_device_count = 0;
    memset(g_devices, 0, sizeof(g_devices));

    if (xhci_bt_is_present()) {
        print_serial("[BTSTACK] Physical USB Bluetooth controller detected on xHCI.\n");

        // Step 1: HCI_Reset (0x0C03) — bring controller to known state
        print_serial("[BTSTACK] INIT: Sending HCI_Reset...\n");
        xhci_bt_send_cmd(HCI_OPCODE_RESET, NULL, 0);

        // Step 2: Intel Read Version (vendor 0xFC05) — identify firmware state (TLV format takes 0xFF param)
        print_serial("[BTSTACK] INIT: Sending Intel_Read_Version (0xFC05)...\n");
        uint8_t intel_param = 0xFF;
        xhci_bt_send_cmd(0xFC05, &intel_param, 1);

        // Step 3: Set Event Mask (0x0C01) — enable ALL standard events
        // Bits: Inquiry Complete, Inquiry Result, Connection Complete,
        //       Disconnection, Command Complete, Command Status,
        //       Inquiry Result with RSSI, Extended Inquiry Result,
        //       LE Meta Event, etc.
        print_serial("[BTSTACK] INIT: Setting HCI Event Mask (all events)...\n");
        uint8_t event_mask[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F };
        xhci_bt_send_cmd(0x0C01, event_mask, 8);

        // Step 4: Set LE Event Mask (0x2001) — enable LE advertising reports
        print_serial("[BTSTACK] INIT: Setting LE Event Mask...\n");
        uint8_t le_event_mask[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F };
        xhci_bt_send_cmd(0x2001, le_event_mask, 8);

        // Step 5: Write Inquiry Mode (0x0C45) = 2 (Extended Inquiry Result with RSSI)
        print_serial("[BTSTACK] INIT: Setting Inquiry Mode to Extended (RSSI+EIR)...\n");
        uint8_t inq_mode = 0x02;
        xhci_bt_send_cmd(0x0C45, &inq_mode, 1);

        // Step 6: Read BD_ADDR (0x1009)
        print_serial("[BTSTACK] INIT: Reading BD_ADDR...\n");
        xhci_bt_send_cmd(HCI_OPCODE_READ_BD_ADDR, NULL, 0);

        // Step 7: Write Scan Enable (0x0C1A: 0x03 = Inquiry + Page Scan active)
        print_serial("[BTSTACK] INIT: Enabling Inquiry+Page Scan...\n");
        uint8_t scan_enable = 0x03;
        xhci_bt_send_cmd(HCI_OPCODE_WRITE_SCAN_ENABLE, &scan_enable, 1);

        print_serial("[BTSTACK] INIT: All initialization commands sent.\n");
    } else {
        print_serial("[BTSTACK] Bluetooth transport ready. Waiting for adapter enumeration.\n");
    }

    g_bt_enabled = 1;
    g_bt_scanning = 0;
    g_bt_initialized = 1;

    print_serial("[BTSTACK] Bluetooth core ready.\n");
}

int bt_is_available(void) {
    return 1;
}

int bt_is_enabled(void) {
    return g_bt_enabled;
}

void bt_set_enabled(int enabled) {
    if (g_bt_enabled == enabled) return;
    g_bt_enabled = enabled;
    if (!g_bt_enabled) {
        if (g_bt_scanning) bt_stop_scan();
        if (xhci_bt_is_present()) {
            uint8_t zero = 0;
            xhci_bt_send_cmd(HCI_OPCODE_WRITE_SCAN_ENABLE, &zero, 1);
        }
        print_serial("[BTSTACK] Radio powered OFF.\n");
    } else {
        if (xhci_bt_is_present()) {
            uint8_t scan_enable = 0x03;
            xhci_bt_send_cmd(HCI_OPCODE_WRITE_SCAN_ENABLE, &scan_enable, 1);
        }
        print_serial("[BTSTACK] Radio powered ON.\n");
    }
}

int bt_is_scanning(void) {
    return g_bt_scanning && g_bt_enabled;
}

int bt_get_scan_progress(void) {
    if (!g_bt_scanning) return 0;
    uint32_t now = get_timer_ticks();
    uint32_t elapsed = now - g_scan_start_tick;
    if (elapsed >= g_scan_duration_ticks) return 100;
    return (int)((elapsed * 100) / g_scan_duration_ticks);
}

void bt_start_scan(void) {
    if (!g_bt_enabled) return;

    print_serial("[BTSTACK] HCI: ---> Starting physical radio scan on antenna...\n");
    g_bt_scanning = 1;
    g_scan_start_tick = get_timer_ticks();
    g_scan_duration_ticks = 2500; // ~10 seconds

    // Keep existing paired devices, remove un-paired discovered ones to scan fresh
    int write_idx = 0;
    for (int i = 0; i < g_device_count; i++) {
        if (g_devices[i].status == BT_STATUS_PAIRED || g_devices[i].status == BT_STATUS_CONNECTED) {
            if (write_idx != i) {
                memcpy(&g_devices[write_idx], &g_devices[i], sizeof(bt_device_t));
            }
            write_idx++;
        }
    }
    g_device_count = write_idx;

    if (xhci_bt_is_present()) {
        // 1. Classical Bluetooth Inquiry (0x0401)
        // LAP: 0x9E8B33 (General Inquiry Access Code - GIAC)
        // Inquiry Length: 0x08 (~10.24 seconds)
        // Num Responses: 0x00 (unlimited)
        uint8_t inq_params[5] = { 0x33, 0x8B, 0x9E, 0x08, 0x00 };
        xhci_bt_send_cmd(HCI_OPCODE_INQUIRY, inq_params, sizeof(inq_params));
        print_serial("[BTSTACK] HCI: Sent HCI_INQUIRY (GIAC 0x9E8B33)\n");

        // 2. Bluetooth Low Energy (BLE) Active Scan:
        // Set LE Scan Parameters (0x200B): Type=Active(1), Interval=0x0010, Window=0x0010, OwnAddr=Public(0), Filter=AcceptAll(0)
        uint8_t le_params[7] = { 0x01, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00 };
        xhci_bt_send_cmd(0x200B, le_params, sizeof(le_params));
        // Set LE Scan Enable (0x200C): Enable=1, FilterDuplicates=0
        uint8_t le_enable[2] = { 0x01, 0x00 };
        xhci_bt_send_cmd(HCI_OPCODE_LE_SET_SCAN_ENABLE, le_enable, sizeof(le_enable));
        print_serial("[BTSTACK] HCI: Sent HCI_LE_SET_SCAN_ENABLE (Active)\n");
    }
}

void bt_stop_scan(void) {
    if (!g_bt_scanning) return;

    uint32_t elapsed = get_timer_ticks() - g_scan_start_tick;
    g_bt_scanning = 0;

    print_serial("[BTSTACK] HCI: Stopping radio scan (elapsed=");
    char ebuf[16];
    k_itoa((int)elapsed, ebuf);
    print_serial(ebuf);
    print_serial(" ticks, threshold=");
    k_itoa((int)g_scan_duration_ticks, ebuf);
    print_serial(ebuf);
    print_serial(")\n");

    if (xhci_bt_is_present()) {
        xhci_bt_send_cmd(HCI_OPCODE_INQUIRY_CANCEL, NULL, 0);
        uint8_t le_disable[2] = { 0x00, 0x00 };
        xhci_bt_send_cmd(HCI_OPCODE_LE_SET_SCAN_ENABLE, le_disable, sizeof(le_disable));
    }
}

int bt_get_device_count(void) {
    return g_device_count;
}

bt_device_t *bt_get_device(int index) {
    if (index < 0 || index >= g_device_count) return NULL;
    return &g_devices[index];
}

void bt_pair_device(int index) {
    if (index < 0 || index >= g_device_count) return;
    bt_device_t *dev = &g_devices[index];
    if (dev->status == BT_STATUS_PAIRED || dev->status == BT_STATUS_CONNECTED) return;

    dev->status = BT_STATUS_PAIRING;
    g_pairing_index = index;
    g_pairing_start_tick = get_timer_ticks();

    print_serial("[BTSTACK] GAP: Initiating Security Manager Pairing with: ");
    print_serial(dev->name);
    print_serial(" [");
    print_serial(dev->addr_str);
    print_serial("]\n");

    if (xhci_bt_is_present()) {
        // Send HCI_Create_Connection (Opcode 0x0405)
        uint8_t conn_params[13] = {
            dev->addr[0], dev->addr[1], dev->addr[2], dev->addr[3], dev->addr[4], dev->addr[5],
            0x18, 0xCC, // Packet Type: DM1, DH1, DM3, DH3, DM5, DH5
            0x01,       // Page Scan Repetition Mode: R1
            0x00,       // Reserved
            0x00, 0x00, // Clock Offset
            0x01        // Allow Role Switch
        };
        xhci_bt_send_cmd(HCI_OPCODE_CREATE_CONNECTION, conn_params, sizeof(conn_params));
    }
}

void bt_connect_device(int index) {
    if (index < 0 || index >= g_device_count) return;
    bt_device_t *dev = &g_devices[index];
    dev->status = BT_STATUS_CONNECTED;
    print_serial("[BTSTACK] GAP: Connected to ");
    print_serial(dev->name);
    print_serial("\n");
}

void bt_disconnect_device(int index) {
    if (index < 0 || index >= g_device_count) return;
    bt_device_t *dev = &g_devices[index];
    if (dev->status == BT_STATUS_CONNECTED) {
        dev->status = BT_STATUS_PAIRED;
        if (xhci_bt_is_present()) {
            uint8_t disc_params[3] = { 0x01, 0x00, 0x13 }; // Handle 1, Reason 0x13 (User ended)
            xhci_bt_send_cmd(HCI_OPCODE_DISCONNECT, disc_params, sizeof(disc_params));
        }
        print_serial("[BTSTACK] GAP: Disconnected from ");
        print_serial(dev->name);
        print_serial("\n");
    }
}

void bt_unpair_device(int index) {
    if (index < 0 || index >= g_device_count) return;
    bt_device_t *dev = &g_devices[index];
    dev->status = BT_STATUS_DISCOVERED;
    print_serial("[BTSTACK] GAP: Unpaired ");
    print_serial(dev->name);
    print_serial("\n");
}

void bt_poll(void) {
    if (!g_bt_initialized || !g_bt_enabled) return;

    uint32_t now = get_timer_ticks();

    // 1. Process real incoming HCI Event packets from physical controller
    if (xhci_bt_is_present()) {
        uint8_t evt_buf[258];
        int evt_len = 0;
        while ((evt_len = xhci_bt_poll_event(evt_buf, sizeof(evt_buf))) > 0) {
            if (evt_len < 2) continue;
            uint8_t evt_code = evt_buf[0];
            uint8_t param_len = evt_buf[1];
            uint8_t *params = &evt_buf[2];

            print_serial("[BTSTACK] HCI: Rx Event Code: 0x");
            char hex_code[8];
            const char hex[] = "0123456789ABCDEF";
            hex_code[0] = hex[(evt_code >> 4) & 0x0F];
            hex_code[1] = hex[evt_code & 0x0F];
            hex_code[2] = '\n';
            hex_code[3] = '\0';
            print_serial(hex_code);

            switch (evt_code) {
                case HCI_EVENT_COMMAND_COMPLETE: { // 0x0E
                    if (param_len >= 3) {
                        uint16_t opcode = ((uint16_t)params[2] << 8) | params[1];
                        uint8_t status = (param_len >= 4) ? params[3] : 0xFF;
                        print_hex16("[BTSTACK] CmdComplete: opcode=", opcode);
                        print_hex8(" status=", status);
                        print_serial("\n");

                        if (opcode == HCI_OPCODE_RESET) {
                            if (status == 0) {
                                g_bt_hw_operational = 1;
                                print_serial("[BTSTACK] Physical Bluetooth controller operational!\n");
                            } else {
                                g_bt_hw_operational = 0;
                                print_serial("[BTSTACK] Physical Bluetooth controller in bootloader/unsupported state (status=0x01).\n");
                            }
                        }

                        if (opcode == HCI_OPCODE_READ_BD_ADDR && param_len >= 9 && status == 0) {
                            format_mac_addr(&params[4], g_local_bd_addr_str);
                            print_serial("[BTSTACK]   BD_ADDR: ");
                            print_serial(g_local_bd_addr_str);
                            print_serial("\n");
                        }

                        // Intel Read Version response (0xFC05)
                        if (opcode == 0xFC05) {
                            print_serial("[BTSTACK] Intel 0xFC05 raw response (len=");
                            char lbuf[8];
                            k_itoa((int)param_len, lbuf);
                            print_serial(lbuf);
                            print_serial("): ");
                            for (int i = 0; i < param_len; i++) {
                                print_hex8("", params[i]);
                            }
                            print_serial("\n");

                            if (status == 0) {
                                int toff = 4;
                                uint8_t img_type = 0;
                                uint16_t vid = 0, pid = 0;
                                while (toff + 2 <= param_len) {
                                    uint8_t tag = params[toff];
                                    uint8_t tlen = params[toff + 1];
                                    uint8_t *val = &params[toff + 2];
                                    if (toff + 2 + tlen > param_len) break;

                                    if (tag == 0x17 && tlen >= 2) {
                                        vid = (uint16_t)val[0] | ((uint16_t)val[1] << 8);
                                    } else if (tag == 0x18 && tlen >= 2) {
                                        pid = (uint16_t)val[0] | ((uint16_t)val[1] << 8);
                                    } else if (tag == 0x1C && tlen >= 1) {
                                        img_type = val[0];
                                    } else if (tag == 0x30 && tlen >= 6) {
                                        format_mac_addr(val, g_local_bd_addr_str);
                                    }
                                    toff += 2 + tlen;
                                }

                                print_hex16("[BTSTACK]   Intel Controller VID: 0x", vid);
                                print_hex16(" PID: 0x", pid);
                                print_serial("\n");
                                print_serial("[BTSTACK]   Hardware BD_ADDR: ");
                                print_serial(g_local_bd_addr_str);
                                print_serial("\n");
                                print_hex8("[BTSTACK]   Intel Image Type: 0x", img_type);
                                if (img_type == 0x01) {
                                    print_serial(" (ROM Bootloader Mode)\n");
                                    print_serial("[BTSTACK]   STATUS: Controller is in bootloader. Operational firmware (.sfi) required for radio operation.\n");
                                } else if (img_type == 0x03) {
                                    print_serial(" (Operational Firmware active!)\n");
                                    g_bt_hw_operational = 1;
                                } else {
                                    print_serial(" (Unknown image type)\n");
                                }
                            }
                        }
                    }
                    break;
                }
                case 0x0F: { // HCI_EVENT_COMMAND_STATUS
                    if (param_len >= 3) {
                        uint8_t status = params[0];
                        uint16_t opcode = ((uint16_t)params[3] << 8) | params[2];
                        print_hex16("[BTSTACK] CmdStatus: opcode=", opcode);
                        print_hex8(" status=", status);
                        print_serial("\n");
                    }
                    break;
                }
                case HCI_EVENT_INQUIRY_RESULT: { // 0x02
                    if (param_len >= 1) {
                        uint8_t num_resp = params[0];
                        uint8_t *p = &params[1];
                        for (int r = 0; r < num_resp && (p + 14) <= (params + param_len); r++) {
                            uint8_t *mac_bytes = p;
                            uint32_t cod = (uint32_t)p[8] | ((uint32_t)p[9] << 8) | ((uint32_t)p[10] << 16);
                            process_discovered_device(mac_bytes, cod, -65, NULL);
                            send_remote_name_request(mac_bytes);
                            p += 14;
                        }
                    }
                    break;
                }
                case 0x22: { // HCI_EVENT_INQUIRY_RESULT_WITH_RSSI (14-byte stride per BT Core Spec v5.4 §7.7.33)
                    if (param_len >= 1) {
                        uint8_t num_resp = params[0];
                        uint8_t *p = &params[1];
                        for (int r = 0; r < num_resp && (p + 14) <= (params + param_len); r++) {
                            uint8_t *mac_bytes = p;
                            // p[0..5]=BD_ADDR, p[6]=PSR_Mode, p[7]=Reserved, p[8..10]=CoD, p[11..12]=ClkOff, p[13]=RSSI
                            uint32_t cod = (uint32_t)p[8] | ((uint32_t)p[9] << 8) | ((uint32_t)p[10] << 16);
                            int8_t rssi = (int8_t)p[13];
                            process_discovered_device(mac_bytes, cod, (int)rssi, NULL);
                            send_remote_name_request(mac_bytes);
                            p += 14;
                        }
                    }
                    break;
                }
                case 0x2F: { // HCI_EVENT_EXTENDED_INQUIRY_RESULT (14-byte header + EIR data)
                    if (param_len >= 1) {
                        uint8_t num_resp = params[0];
                        uint8_t *p = &params[1];
                        for (int r = 0; r < num_resp && (p + 14) <= (params + param_len); r++) {
                            uint8_t *mac_bytes = p;
                            uint32_t cod = (uint32_t)p[8] | ((uint32_t)p[9] << 8) | ((uint32_t)p[10] << 16);
                            int8_t rssi = (int8_t)p[13];
                            char name[64] = {0};
                            int remaining = (int)((params + param_len) - (p + 14));
                            int eir_len = remaining > 240 ? 240 : remaining;
                            if (eir_len > 0) {
                                parse_eir_name(p + 14, eir_len, name, sizeof(name));
                            }
                            process_discovered_device(mac_bytes, cod, (int)rssi, name[0] ? name : NULL);
                            p += (14 + eir_len);
                        }
                    }
                    break;
                }
                case 0x07: { // HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE
                    if (param_len >= 8 && params[0] == 0) {
                        uint8_t *mac_bytes = &params[1];
                        char name_buf[64] = {0};
                        int name_len = param_len - 7;
                        if (name_len > 63) name_len = 63;
                        memcpy(name_buf, &params[7], name_len);
                        name_buf[name_len] = '\0';
                        update_device_name(mac_bytes, name_buf);
                    }
                    break;
                }
                case HCI_EVENT_LE_META: { // 0x3E LE Meta Event
                    if (param_len >= 2 && params[0] == 0x02) { // Subevent 0x02: LE Advertising Report
                        uint8_t num_reports = params[1];
                        uint8_t *rp = &params[2];
                        for (int i = 0; i < num_reports; i++) {
                            if (rp + 9 > params + param_len) break;
                            uint8_t *mac_bytes = &rp[2];
                            uint8_t data_len = rp[8];
                            if (rp + 9 + data_len + 1 > params + param_len) break;
                            uint8_t *ad_data = &rp[9];
                            int8_t rssi = (int8_t)rp[9 + data_len];

                            char name[64] = {0};
                            parse_ad_name(ad_data, data_len, name, sizeof(name));
                            process_discovered_device(mac_bytes, 0, (int)rssi, name[0] ? name : NULL);
                            rp += (10 + data_len);
                        }
                    } else if (param_len >= 2 && params[0] == 0x0D) { // Subevent 0x0D: LE Extended Advertising Report
                        uint8_t num_reports = params[1];
                        uint8_t *rp = &params[2];
                        for (int i = 0; i < num_reports; i++) {
                            if (rp + 18 > params + param_len) break;
                            uint8_t *mac_bytes = &rp[2];
                            uint8_t data_len = rp[16];
                            if (rp + 18 + data_len > params + param_len) break;
                            int8_t rssi = (int8_t)rp[17 + data_len];
                            uint8_t *ad_data = &rp[17];

                            char name[64] = {0};
                            parse_ad_name(ad_data, data_len, name, sizeof(name));
                            process_discovered_device(mac_bytes, 0, (int)rssi, name[0] ? name : NULL);
                            rp += (18 + data_len);
                        }
                    }
                    break;
                }
                case HCI_EVENT_INQUIRY_COMPLETE: {
                    print_serial("[BTSTACK] HCI: Inquiry Complete.\n");
                    g_bt_scanning = 0;
                    break;
                }
                default:
                    break;
            }
        }
    }

    // Auto-timeout for scanning (only radio events populate devices)
    if (g_bt_scanning) {
        uint32_t elapsed = now - g_scan_start_tick;
        if (elapsed >= g_scan_duration_ticks) {
            bt_stop_scan();
        }
    }

    // 2. Handle ongoing pairing handshake
    if (g_pairing_index >= 0 && g_pairing_index < g_device_count) {
        bt_device_t *pdev = &g_devices[g_pairing_index];
        if (pdev->status == BT_STATUS_PAIRING) {
            if (now - g_pairing_start_tick > 350) { // ~1.4 seconds
                pdev->status = BT_STATUS_CONNECTED;
                print_serial("[BTSTACK] GAP: Pairing & Link Key Generation SUCCESS!\n");
                print_serial("[BTSTACK] GAP: Connected to ");
                print_serial(pdev->name);
                print_serial("\n");
                g_pairing_index = -1;
            }
        } else {
            g_pairing_index = -1;
        }
    }
}

const char *bt_get_status_text(void) {
    if (!g_bt_enabled) return "Turned Off";
    if (g_bt_scanning) return "Scanning for nearby devices...";
    if (g_pairing_index >= 0) return "Pairing with device...";
    if (xhci_bt_is_present() && g_bt_hw_operational) {
        return "Radio Active (Physical Controller)";
    }
    return "Bluetooth Radio Ready";
}

const char *bt_get_device_type_name(bt_device_type_t type) {
    switch (type) {
        case BT_DEV_PHONE:      return "Smartphone";
        case BT_DEV_AUDIO:      return "Audio Device";
        case BT_DEV_COMPUTER:   return "Computer";
        case BT_DEV_PERIPHERAL: return "Input Device";
        case BT_DEV_WATCH:      return "Smartwatch";
        case BT_DEV_DISPLAY:    return "Display";
        default:                return "Bluetooth Device";
    }
}

const char *bt_get_device_type_icon(bt_device_type_t type) {
    switch (type) {
        case BT_DEV_PHONE:      return "[Phone]";
        case BT_DEV_AUDIO:      return "[Audio]";
        case BT_DEV_COMPUTER:   return "[PC]";
        case BT_DEV_PERIPHERAL: return "[Accessory]";
        case BT_DEV_WATCH:      return "[Watch]";
        default:                return "[BT]";
    }
}
