import os
import struct

def create_fat_data_disk():
    # 10MB Disk (matches cylinders=20, heads=16, spt=63 exactly)
    disk_size = 10321920
    image = bytearray(disk_size)
    
    # 1. FAT16 Boot Sector (BPB)
    # Minimal FAT16 BPB
    image[0:3] = b'\xeb\x3c\x90' # JMP
    image[3:11] = b'MSDOS5.0'    # OEM Name
    struct.pack_into('<H', image, 11, 512)   # Bytes per sector
    image[13] = 8                            # Sectors per cluster (4KB clusters)
    struct.pack_into('<H', image, 14, 1)     # Reserved sectors
    image[16] = 2                            # Number of FATs
    struct.pack_into('<H', image, 17, 512)   # Root entries
    struct.pack_into('<H', image, 19, 0)     # Total sectors (small)
    image[21] = 0xf8                         # Media descriptor (Fixed disk)
    struct.pack_into('<H', image, 22, 32)    # Sectors per FAT
    struct.pack_into('<H', image, 24, 63)    # Sectors per track
    struct.pack_into('<H', image, 26, 16)    # Number of heads
    struct.pack_into('<I', image, 28, 0)     # Hidden sectors
    struct.pack_into('<I', image, 32, disk_size // 512) # Total sectors (large)
    
    # Extended BPB
    image[36] = 0x80 # Drive number
    image[38] = 0x29 # Signature
    image[39:43] = b'\x12\x34\x56\x78' # Serial
    image[43:54] = b'PURE_DATA  '      # Label
    image[54:62] = b'FAT16   '         # System ID
    
    # Boot signature
    image[510] = 0x55
    image[511] = 0xaa
    
    # FAT tables (Sector 1 and onwards)
    # FAT16 Entry 0: F8FF, Entry 1: FFFF
    struct.pack_into('<H', image, 512, 0xfff8)
    struct.pack_into('<H', image, 514, 0xffff)
    
    # Root Directory (starts after Reserved + 2*FAT_size)
    # Root starts at sector 1 + 2*32 = 65
    root_start = 65 * 512
    
    # Add WALKTHROUGH.MPG to root
    filename = "walkthrough.mpg"
    if os.path.exists(filename):
        with open(filename, 'rb') as f:
            video_data = f.read()
            
        # Root Entry
        # Name (8.3)
        image[root_start : root_start+11] = b'WALKTHRO   '
        image[root_start+8:root_start+11] = b'MPG'
        image[root_start+11] = 0x20 # Archive attribute
        
        # First cluster (Cluster 2 is the first data cluster)
        struct.pack_into('<H', image, root_start+26, 2)
        # Size
        struct.pack_into('<I', image, root_start+28, len(video_data))
        
        # Write video data to Cluster 2
        # Data starts after Root Dir (512 entries * 32 bytes = 16KB = 32 sectors)
        # Data start = 65 + 32 = 97
        data_start = 97 * 512
        image[data_start : data_start+len(video_data)] = video_data
        
        # Update FAT for video data
        # How many clusters?
        sectors_needed = (len(video_data) + 511) // 512
        clusters_needed = (sectors_needed + 7) // 8
        
        for i in range(clusters_needed):
            cluster = 2 + i
            if i == clusters_needed - 1:
                next_val = 0xffff # EOF
            else:
                next_val = cluster + 1
            # FAT1
            struct.pack_into('<H', image, 512 + cluster*2, next_val)
            # FAT2
            struct.pack_into('<H', image, 512 + 32*512 + cluster*2, next_val)

    with open('pure_data.img', 'wb') as f:
        f.write(image)
    print(f"Created pure_data.img ({len(image)} bytes)")

if __name__ == '__main__':
    if os.path.exists('pure_data.img'):
        print("pure_data.img already exists, skipping creation to preserve persistent data.")
    else:
        create_fat_data_disk()
