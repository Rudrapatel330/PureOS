import struct
import os

with open('disk.img', 'rb') as f:
    # Read the entire disk
    disk = f.read()
    
    print("Searching for kernel.bin on disk...")
    
    # Method 2: Look for FAT12 root directory entries
    print("\nChecking FAT12 structure...")
    
    # Read BPB from boot sector
    f.seek(11)  # Start of BPB
    bytes_per_sector = struct.unpack('<H', f.read(2))[0]
    sectors_per_cluster = struct.unpack('<B', f.read(1))[0]
    reserved_sectors = struct.unpack('<H', f.read(2))[0]
    fat_count = struct.unpack('<B', f.read(1))[0]
    root_entries = struct.unpack('<H', f.read(2))[0]
    total_sectors = struct.unpack('<H', f.read(2))[0]
    media_descriptor = struct.unpack('<B', f.read(1))[0]
    sectors_per_fat = struct.unpack('<H', f.read(2))[0]
    
    print(f"BPB Info:")
    print(f"  Bytes per sector: {bytes_per_sector}")
    print(f"  Reserved sectors: {reserved_sectors}")
    print(f"  FAT count: {fat_count}")
    print(f"  Root entries: {root_entries}")
    print(f"  Sectors per FAT: {sectors_per_fat}")
    
    # Calculate FAT12 structure
    fat_start = reserved_sectors
    root_start = fat_start + (fat_count * sectors_per_fat)
    root_sectors = (root_entries * 32 + bytes_per_sector - 1) // bytes_per_sector
    data_start = root_start + root_sectors
    
    print(f"\nFAT12 Layout:")
    print(f"  FAT1 starts at LBA: {fat_start}")
    print(f"  Root directory starts at LBA: {root_start}")
    print(f"  Data area starts at LBA: {data_start}")
    
    # Read root directory
    f.seek(root_start * 512)
    for i in range(root_entries):
        entry = f.read(32)
        if entry[0] == 0x00:  # Free entry
            break
        if entry[0] != 0xE5:  # Not deleted
            name = entry[0:8].decode('ascii', errors='ignore').strip()
            ext = entry[8:11].decode('ascii', errors='ignore').strip()
            if name:
                first_cluster = struct.unpack('<H', entry[26:28])[0]
                file_size = struct.unpack('<I', entry[28:32])[0]
                
                # Calculate LBA for cluster
                # Data area starts at cluster 2
                cluster_lba = data_start + (first_cluster - 2) * sectors_per_cluster
                
                print(f"\nFile: {name}.{ext}")
                print(f"  First cluster: {first_cluster}")
                print(f"  Size: {file_size} bytes")
                print(f"  Starting LBA: {cluster_lba}")
                
                track = cluster_lba // 36
                head = (cluster_lba // 18) % 2
                sector = (cluster_lba % 18) + 1
                print(f"  CHS: Track={track}, Head={head}, Sector={sector}")
