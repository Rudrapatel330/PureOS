import struct
import os

print("=== DISK IMAGE ANALYSIS ===\n")

filename = 'disk.img'
if not os.path.exists(filename):
    print(f"Error: {filename} not found!")
    exit(1)

with open(filename, 'rb') as f:
    # 1. Check boot sector
    print("1. BOOT SECTOR (LBA 0):")
    f.seek(0)
    boot = f.read(512)
    
    # Check boot signature
    if boot[510:512] == b'\x55\xAA':
        print("   ✓ Boot signature: 0x55AA")
    else:
        print("   ✗ NO BOOT SIGNATURE!")
    
    # Read BPB
    f.seek(11)
    bytes_per_sector = struct.unpack('<H', f.read(2))[0]
    sectors_per_cluster = struct.unpack('<B', f.read(1))[0]
    reserved_sectors = struct.unpack('<H', f.read(2))[0]
    fat_count = struct.unpack('<B', f.read(1))[0]
    root_entries = struct.unpack('<H', f.read(2))[0]
    total_sectors = struct.unpack('<H', f.read(2))[0] # This matches offset 19 (0x13)
    media_descriptor = struct.unpack('<B', f.read(1))[0]
    sectors_per_fat = struct.unpack('<H', f.read(2))[0]
    
    print(f"   Reserved sectors: {reserved_sectors}")
    print(f"   FAT count: {fat_count}")
    print(f"   Root entries: {root_entries}")
    print(f"   Sectors per FAT: {sectors_per_fat}")
    
    # 2. Calculate FAT12 layout
    fat_start = reserved_sectors
    root_start = fat_start + (fat_count * sectors_per_fat)
    root_size_sectors = (root_entries * 32 + bytes_per_sector - 1) // bytes_per_sector
    data_start = root_start + root_size_sectors
    
    print(f"\n2. FAT12 LAYOUT:")
    print(f"   FAT1 starts at LBA: {fat_start}")
    print(f"   Root directory starts at LBA: {root_start}")
    print(f"   Data area starts at LBA: {data_start} (Cluster 2)")
    
    # 3. Read root directory
    print(f"\n3. ROOT DIRECTORY ENTRIES:")
    f.seek(root_start * 512)
    found_kernel = False
    
    for i in range(min(root_entries, 20)):  # Check first 20 entries
        entry = f.read(32)
        
        if entry[0] == 0x00:  # Free entry
            break
            
        if entry[0] != 0xE5:  # Not deleted
            name = entry[0:8].decode('ascii', 'ignore').strip()
            ext = entry[8:11].decode('ascii', 'ignore').strip()
            
            if name:  # Valid entry
                attr = entry[11]
                first_cluster = struct.unpack('<H', entry[26:28])[0]
                file_size = struct.unpack('<I', entry[28:32])[0]
                
                print(f"   [{i}] {name}.{ext}")
                print(f"       Attr: 0x{attr:02X}, Cluster: {first_cluster}, Size: {file_size}")
                
                # Check for KERNEL.BIN or similar (case insensitive)
                full_name = f"{name}.{ext}".upper()
                if "KERNEL" in full_name:
                    found_kernel = True
                    kernel_cluster = first_cluster
                    kernel_size = file_size
                    
                    # Calculate LBA
                    kernel_lba = data_start + (first_cluster - 2) * sectors_per_cluster
                    sectors_needed = (file_size + bytes_per_sector - 1) // bytes_per_sector
                    
                    print(f"\n   ✓ FOUND KERNEL.BIN!")
                    print(f"       At cluster: {first_cluster}")
                    print(f"       Starting LBA: {kernel_lba}")
                    print(f"       Size: {file_size} bytes ({sectors_needed} sectors)")
                    
                    # Calculate CHS
                    total_sectors_per_cylinder = 18 * 2  # 18 sectors/track × 2 heads
                    lba = kernel_lba
                    cylinder = lba // total_sectors_per_cylinder
                    head = (lba // 18) % 2
                    sector = (lba % 18) + 1
                    
                    print(f"       CHS: Cylinder={cylinder}, Head={head}, Sector={sector}")
    
    if not found_kernel:
        print("\n   ✗ KERNEL.BIN NOT FOUND IN ROOT DIRECTORY!")
        
        # Search for any .BIN file
        print("\n   Searching for any file...")
        f.seek(root_start * 512)
        # Reset seeking for full scan if needed here, but loop above did basic check.
    
    # 4. Dump first few sectors of data area to see what's actually there
    print(f"\n4. DATA AREA PROBE (first 5 sectors starting at LBA {data_start}):")
    f.seek(data_start * 512)
    
    for i in range(5):
        try:
            sector_data = f.read(512)
            if not sector_data: break
            first_32 = sector_data[:32]
            
            # Check if this looks like code
            is_code = False
            code_patterns = [
                b'\x55\x89\xE5',  # push ebp; mov ebp, esp
                b'\xB8',          # mov eax, ...
                b'\xE8',          # call
                b'\xC3',          # ret
                b'\x7FELF'        # ELF
            ]
            
            for pattern in code_patterns:
                if pattern in first_32:
                    is_code = True
                    break
            
            hex_dump = first_32.hex()
            if len(hex_dump) > 64: hex_dump = hex_dump[:64] + "..."
            
            non_zero = first_32 != b'\x00' * 32
            
            if non_zero:
                print(f"   LBA {data_start + i}: {hex_dump}")
                if is_code:
                    print(f"        ↑ Looks like code/ELF!")
        except Exception as e:
            print(f"Error reading sector: {e}")

    print("\n=== END ANALYSIS ===")
