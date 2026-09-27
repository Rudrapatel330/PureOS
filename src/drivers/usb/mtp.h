#ifndef _USB_MTP_H_
#define _USB_MTP_H_

#include <stdint.h>
#include <stddef.h>

// MTP Container Types
#define MTP_CONTAINER_UNDEFINED   0x0000
#define MTP_CONTAINER_COMMAND     0x0001
#define MTP_CONTAINER_DATA        0x0002
#define MTP_CONTAINER_RESPONSE    0x0003
#define MTP_CONTAINER_EVENT       0x0004

// MTP Operations (USB Still Image / PTP & MTP Spec)
#define MTP_OP_GET_DEVICE_INFO    0x1001
#define MTP_OP_OPEN_SESSION       0x1002
#define MTP_OP_CLOSE_SESSION      0x1003
#define MTP_OP_GET_STORAGE_IDS    0x1004
#define MTP_OP_GET_STORAGE_INFO   0x1005
#define MTP_OP_GET_NUM_OBJECTS    0x1006
#define MTP_OP_GET_OBJECT_HANDLES 0x1007
#define MTP_OP_GET_OBJECT_INFO    0x1008
#define MTP_OP_GET_OBJECT         0x1009
#define MTP_OP_DELETE_OBJECT      0x100B
#define MTP_OP_SEND_OBJECT_INFO   0x100C
#define MTP_OP_SEND_OBJECT        0x100D
#define MTP_OP_MOVE_OBJECT        0x1019
#define MTP_OP_COPY_OBJECT        0x101A

// MTP Responses
#define MTP_RESP_UNDEFINED               0x2000
#define MTP_RESP_OK                      0x2001
#define MTP_RESP_GENERAL_ERROR           0x2002
#define MTP_RESP_SESSION_NOT_OPEN        0x2003
#define MTP_RESP_INVALID_TRANSACTION_ID  0x2004
#define MTP_RESP_OPERATION_NOT_SUPPORTED 0x2005
#define MTP_RESP_PARAMETER_NOT_SUPPORTED 0x2006
#define MTP_RESP_INCOMPLETE_TRANSFER     0x2007
#define MTP_RESP_INVALID_STORAGE_ID      0x2008
#define MTP_RESP_INVALID_OBJECT_HANDLE   0x2009
#define MTP_RESP_STORE_FULL              0x200C
#define MTP_RESP_STORE_READ_ONLY         0x200E
#define MTP_RESP_SESSION_ALREADY_OPEN    0x201E

// MTP Object Formats
#define MTP_FORMAT_UNDEFINED             0x3000
#define MTP_FORMAT_ASSOCIATION           0x3001 // Directory / Folder
#define MTP_FORMAT_TEXT                  0x3004
#define MTP_FORMAT_HTML                  0x3005
#define MTP_FORMAT_MP3                   0x3009
#define MTP_FORMAT_EXIF_JPEG             0x3801
#define MTP_FORMAT_PNG                   0x380B
#define MTP_FORMAT_MP4                   0xB982

// MTP Standard 12-byte Container Header
typedef struct __attribute__((packed)) {
    uint32_t length;          // Total container length in bytes (including this header)
    uint16_t container_type;  // Command (1), Data (2), Response (3), Event (4)
    uint16_t code;            // Operation or Response code
    uint32_t transaction_id;  // Unique sequence transaction ID
} mtp_header_t;

// Decoded phone file/directory entry
typedef struct {
    uint32_t handle;
    uint32_t storage_id;
    uint16_t format;
    uint32_t size_bytes;
    int is_dir;
    char name[128];
} mtp_object_entry_t;

// Public Driver API
int mtp_is_connected(void);
int mtp_rescan_if_needed(void);
void mtp_poll_hotplug(void);
int mtp_open_session(void);
int mtp_close_session(void);
int mtp_get_storage_id(uint32_t *storage_id, char *storage_desc, int max_desc);
typedef int (*mtp_entry_callback_t)(mtp_object_entry_t *entry, int index, int total, void *user_data);
int mtp_list_directory(uint32_t parent_handle, mtp_object_entry_t *entries, int max_entries);
int mtp_list_directory_streaming(uint32_t parent_handle, mtp_object_entry_t *entries, int max_entries,
                                 mtp_entry_callback_t cb, void *user_data);
int mtp_fetch_handles(uint32_t parent_handle, uint32_t **out_handles, int *out_count, uint32_t *out_st_id);
int mtp_fetch_single_object_info(uint32_t handle, uint32_t storage_id, mtp_object_entry_t *entry);
int mtp_pull_file(uint32_t object_handle, const char *local_dest_path);
int mtp_push_file(const char *local_src_path, uint32_t parent_handle, const char *phone_filename);
int mtp_delete_object(uint32_t object_handle);
int mtp_move_object(uint32_t object_handle, uint32_t target_parent_handle);
int mtp_copy_object(uint32_t object_handle, uint32_t target_parent_handle);

#endif // _USB_MTP_H_
