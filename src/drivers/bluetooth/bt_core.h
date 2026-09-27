#ifndef BT_CORE_H
#define BT_CORE_H

#include "../../kernel/types.h"
#include "btstack_config.h"

// Bluetooth BD_ADDR (6 bytes)
typedef uint8_t bd_addr_t[6];

// HCI Standard Command Opcodes
#define HCI_OPCODE_INQUIRY                  0x0401
#define HCI_OPCODE_INQUIRY_CANCEL           0x0402
#define HCI_OPCODE_CREATE_CONNECTION        0x0405
#define HCI_OPCODE_DISCONNECT               0x0406
#define HCI_OPCODE_RESET                    0x0C03
#define HCI_OPCODE_WRITE_SCAN_ENABLE        0x0C1A
#define HCI_OPCODE_READ_BD_ADDR             0x1009
#define HCI_OPCODE_LE_SET_SCAN_ENABLE       0x200C

// HCI Standard Event Codes
#define HCI_EVENT_INQUIRY_COMPLETE          0x01
#define HCI_EVENT_INQUIRY_RESULT            0x02
#define HCI_EVENT_CONNECTION_COMPLETE       0x03
#define HCI_EVENT_DISCONNECTION_COMPLETE    0x05
#define HCI_EVENT_COMMAND_COMPLETE          0x0E
#define HCI_EVENT_COMMAND_STATUS            0x0F
#define HCI_EVENT_LE_META                   0x3E

// Device Category / Class
typedef enum {
    BT_DEV_UNKNOWN = 0,
    BT_DEV_PHONE,        // Smartphone / Tablet
    BT_DEV_AUDIO,        // Headphones / Earbuds / Speaker
    BT_DEV_COMPUTER,     // Laptop / Desktop PC
    BT_DEV_PERIPHERAL,   // Mouse / Keyboard / Gamepad
    BT_DEV_WATCH,        // Smartwatch / Wearable
    BT_DEV_DISPLAY       // Smart TV / Wireless Display
} bt_device_type_t;

// Connection / Pairing Status
typedef enum {
    BT_STATUS_DISCOVERED = 0,
    BT_STATUS_PAIRING,
    BT_STATUS_PAIRED,
    BT_STATUS_CONNECTING,
    BT_STATUS_CONNECTED,
    BT_STATUS_DISCONNECTED
} bt_device_status_t;

// Bluetooth Device Entry
typedef struct {
    bd_addr_t addr;
    char addr_str[20];       // Formatted "XX:XX:XX:XX:XX:XX"
    char name[64];           // Friendly name (e.g. "iPhone 15 Pro")
    bt_device_type_t type;   // Audio, Phone, PC, etc.
    int rssi;                // RSSI in dBm (-30 to -95)
    int signal_bars;         // 1 to 4 signal strength bars
    bt_device_status_t status;
    uint32_t class_of_device;// CoD value from Inquiry
    uint32_t discovery_tick;
    uint32_t last_seen_tick;
    int is_active;
} bt_device_t;

// Bluetooth Subsystem API
void bt_init(void);
int bt_is_available(void);
int bt_is_enabled(void);
void bt_set_enabled(int enabled);

int bt_is_scanning(void);
void bt_start_scan(void);
void bt_stop_scan(void);
int bt_get_scan_progress(void); // 0 to 100 percentage

int bt_get_device_count(void);
bt_device_t *bt_get_device(int index);

void bt_pair_device(int index);
void bt_connect_device(int index);
void bt_disconnect_device(int index);
void bt_unpair_device(int index);

void bt_poll(void);
const char *bt_get_status_text(void);
const char *bt_get_device_type_name(bt_device_type_t type);
const char *bt_get_device_type_icon(bt_device_type_t type);

#endif // BT_CORE_H
