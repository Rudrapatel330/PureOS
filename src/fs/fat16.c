#include "fat16.h"
#include "../drivers/ata.h"
#include "../drivers/vga.h"
#include "../kernel/mem.h"

void fat16_init() {
    /* TODO: Read boot sector and populate BPB */
}

void fat16_list_root() {
    /* Assume generic values for testing since we don't have a real image */
    /* Root dir starts after Reserved + FATs. */
    /* For a standard floppy:
       Reserved = 1
       FATs = 2 * 9 = 18
       Start = 19
    */
    /* We'll try to read sector 19 and see if we find anything valid or just junk from our kernel image */
    
    u8* buf = (u8*) kmalloc(512);
    ata_read_sectors(19, 1, buf);
    
    fat_dir_entry_t* dir = (fat_dir_entry_t*) buf;
    
    kprint("Files:\n");
    
    for (int i = 0; i < 16; i++) { // 512 / 32 = 16 entries per sector
        if (dir[i].filename[0] == 0) break; // No more entries
        if (dir[i].filename[0] == 0xE5) continue; // Deleted
        
        char name[12];
        memory_copy((char*)dir[i].filename, name, 8);
        name[8] = '.';
        memory_copy((char*)dir[i].ext, name+9, 3);
        name[12] = 0;
        
        kprint("  ");
        kprint(name);
        kprint("\n");
    }
    
    kfree(buf);
}
