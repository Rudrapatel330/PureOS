#include "editor.h"
#include "../drivers/keyboard.h"
#include "../fs/fat12.h"
#include "string.h"
#include <stdint.h>

static char editor_buffer[1024]; // 1KB Buffer
static int cursor_pos = 0;

void clear_screen() {
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    for(int i = 0; i < 80*25; i++) vga[i] = 0x0700 | ' ';
    reset_cursor();
}

void start_editor(char* filename) {
    clear_screen();
    print_str("PURE EDITOR v1.0 - Editing: ");
    print_str(filename);
    print_str("\n[Type lines of text. Type '~' on a NEW LINE to Save & Exit]\n");
    print_str("----------------------------------------------------------\n");
    
    // Reset buffer
    memset(editor_buffer, 0, 1024);
    cursor_pos = 0;
    
    // Existing file? Read it first
    if (fs_read(filename, (uint8_t*)editor_buffer)) {
        print_str(editor_buffer);
        cursor_pos = strlen(editor_buffer);
    }
    
    while(1) {
        char* kb_buf = keyboard_get_buffer();
        if (kb_buf && kb_buf[0] != 0) {
            // We need to handle raw input, not line buffered
            // CURRENTLY keyboard.c buffers until Enter.
            // We need a way to get *raw raw* input or just accept line by line?
            // "edit" usually means character-by-character.
            // My keyboard driver is line-buffered.
            // WORKAROUND: For now, we will assume the user types lines and presses enter.
            // BUT that won't capture '~' mid-line unless we change keyboard driver 
            // OR we just poll 0x60 port directly here?
            // Polling 0x60 is racy with interrupt handler.
            // Let's modify keyboard.c to expose "last_char" or just wait for lines.
            
            // Actually, let's keep it simple: Line based editor for now?
            // No, the user wants "Text Editor".
            // Let's use `keyboard_handler` but we need to intercept keys.
            
            // HACK: We will just wait for the buffer to fill (Enter press).
            // A better way is to make keyboard driver have a "raw mode" or callback.
            
            // Let's stick to line-based for this step to avoid rewriting keyboard driver fully.
            // User types a line, hits Enter -> Appends to buffer.
            // User types "~" and hits Enter -> Save.
            
            if (strcmp(kb_buf, "~") == 0) {
                // Save
                if (fs_write(filename, (uint8_t*)editor_buffer, cursor_pos)) {
                    print_str("\n[Saved]\n");
                } else {
                    print_str("\n[Save Failed]\n");
                }
                keyboard_reset_buffer(); // CRITICAL: Clear "~" from buffer!
                return; // Exit
            }
            
            // Append line
            int len = strlen(kb_buf);
            if (cursor_pos + len + 1 < 1024) {
                // Copy line
                for(int i=0; i<len; i++) editor_buffer[cursor_pos++] = kb_buf[i];
                editor_buffer[cursor_pos++] = '\n'; // Add newline
                
                // Echo is already done by keyboard driver?
                // Yes, keyboard driver echoes chars.
                // We just need to ensure the newline is printed (it is by keyboard driver).
            } else {
                print_str("\n[Buffer Full]\n");
            }
            
            keyboard_reset_buffer();
        }
    }
}
