#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * System-wide Constants
 * 
 * This header defines all magic numbers and system constants
 * to improve code readability and maintainability.
 */

// ===== MEMORY ADDRESSES =====
#define VGA_TEXT_BUFFER         0xB8000
#define BOOTLOADER_ADDR         0x7C00
#define STAGE2_LOAD_ADDR        0x7E00
#define FRAMEBUFFER_PHYS_ADDR   0xE0000000
#define BACKBUFFER_ADDR         0x800000
#define VBE_INFO_ADDR           0x6000

// Heap configuration
#define HEAP_START              0x1000000    // 16MB
#define HEAP_SIZE               0xE00000     // 14MB (16MB - 2MB for kernel)
#define MIN_BLOCK_SIZE          16
#define MEMORY_ALIGNMENT        4            // 4-byte alignment

// ===== SCREEN CONFIGURATION =====
#define SCREEN_WIDTH            1024
#define SCREEN_HEIGHT           768
#define SCREEN_BPP              16
#define TASKBAR_HEIGHT          30
#define DESKTOP_HEIGHT          (SCREEN_HEIGHT - TASKBAR_HEIGHT)

// ===== COLORS (16-bit RGB565 format) =====
#define COLOR_WHITE             0xFFFF
#define COLOR_BLACK             0x0000
#define COLOR_RED               0xF800
#define COLOR_GREEN             0x07E0
#define COLOR_BLUE              0x001F
#define COLOR_YELLOW            0xFFE0
#define COLOR_CYAN              0x07FF
#define COLOR_MAGENTA           0xF81F

// Desktop colors
#define COLOR_DESKTOP_BG        0xC618       // Light gray-blue
#define COLOR_WINDOW_BORDER     0x8410       // Gray
#define COLOR_TITLEBAR          0x0000       // Black
#define COLOR_TASKBAR           0xC0C0       // Light gray

// ===== FILESYSTEM LIMITS =====
#define FS_MAX_FILES            128
#define MAX_FILENAME            32
#define MAX_FILE_SIZE           (1024 * 1024)  // 1MB per file

// ===== GUI CONSTANTS =====
#define MAX_WINDOWS             32
#define MAX_ICONS               10
#define ICON_WIDTH              40
#define ICON_HEIGHT             60
#define WINDOW_MIN_WIDTH        100
#define WINDOW_MIN_HEIGHT       80

// ===== INPUT LIMITS =====
#define MAX_CMD_LENGTH          256
#define KEYBOARD_BUFFER_SIZE    128
#define MOUSE_BUFFER_SIZE       32

// ===== HARDWARE PORTS =====
// Keyboard/Mouse (PS/2)
#define PS2_DATA_PORT           0x60
#define PS2_STATUS_PORT         0x64
#define PS2_COMMAND_PORT        0x64

// PIC (Programmable Interrupt Controller)
#define PIC1_COMMAND            0x20
#define PIC1_DATA               0x21
#define PIC2_COMMAND            0xA0
#define PIC2_DATA               0xA1

// ATA (IDE) Primary Bus
#define ATA_DATA                0x1F0
#define ATA_ERROR               0x1F1
#define ATA_FEATURES            0x1F1
#define ATA_SECTOR_COUNT        0x1F2
#define ATA_LBA_LO              0x1F3
#define ATA_LBA_MID             0x1F4
#define ATA_LBA_HI              0x1F5
#define ATA_DRIVE_HEAD          0x1F6
#define ATA_STATUS              0x1F7
#define ATA_COMMAND             0x1F7

// ATA Status Flags
#define ATA_SR_BSY              0x80    // Busy
#define ATA_SR_DRDY             0x40    // Drive ready
#define ATA_SR_DF               0x20    // Drive fault
#define ATA_SR_DSC              0x10    // Drive seek complete
#define ATA_SR_DRQ              0x08    // Data request ready
#define ATA_SR_CORR             0x04    // Corrected data
#define ATA_SR_IDX              0x02    // Index
#define ATA_SR_ERR              0x01    // Error

// Timer (PIT)
#define PIT_CHANNEL0            0x40
#define PIT_COMMAND             0x43
#define PIT_FREQUENCY           1193180

// RTC
#define RTC_COMMAND             0x70
#define RTC_DATA                0x71

// ===== BOOTLOADER CONSTANTS =====
#define BOOT_SIGNATURE          0xAA55
#define SECTORS_PER_TRACK       18
#define HEADS_PER_CYLINDER      2
#define BYTES_PER_SECTOR        512
#define STAGE2_SECTORS          100      // Load 100 sectors (50KB)

// ===== MAGIC NUMBERS =====
#define MULTIBOOT_MAGIC         0x2BADB002
#define GDT_ENTRIES             3
#define IDT_ENTRIES             256

// ===== FONT =====
#define FONT_WIDTH              8
#define FONT_HEIGHT             8

#endif // CONSTANTS_H
