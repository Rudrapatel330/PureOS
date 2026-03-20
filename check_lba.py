import struct

with open('disk.img', 'rb') as f:
    # Read FAT12 structures
    f.seek(11)  # Skip to BPB
    bytes_per_sector = struct.unpack('<H', f.read(2))[0]
    sectors_per_cluster = struct.unpack('<B', f.read(1))[0]
    reserved_sectors = struct.unpack('<H', f.read(2))[0]
    fat_count = struct.unpack('<B', f.read(1))[0]
    root_entries = struct.unpack('<H', f.read(2))[0]
    sectors_per_fat = struct.unpack('<H', f.read(2))[0]
    
    print(f'Bytes per sector: {bytes_per_sector}')
    print(f'Reserved sectors: {reserved_sectors}')
    print(f'FAT count: {fat_count}')
    print(f'Root entries: {root_entries}')
    print(f'Sectors per FAT: {sectors_per_fat}')
    
    # Calculate kernel LBA
    fat_start = reserved_sectors
    # Size of one FAT in sectors
    fat_size_sectors = sectors_per_fat
    
    # Total FAT size
    total_fat_size = fat_count * fat_size_sectors
    
    # Root directory start
    root_start = fat_start + total_fat_size
    
    # Root directory size in sectors
    root_dir_size_bytes = root_entries * 32
    root_sectors = (root_dir_size_bytes + bytes_per_sector - 1) // bytes_per_sector
    
    # Data area start
    data_start = root_start + root_sectors
    
    print(f'FAT1 Start: {fat_start}')
    print(f'Root Start: {root_start}')
    print(f'Data Start: {data_start}')
    print(f'Kernel should be at LBA: {data_start} (cluster 2)')
