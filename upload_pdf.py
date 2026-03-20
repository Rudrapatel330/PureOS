import os
import struct
import sys

def upload_to_pure_data(file_path):
    if not os.path.exists(file_path):
        print(f"Error: {file_path} not found.")
        return

    disk_path = 'pureos.img'
    disk_offset = 50 * 1024 * 1024 # 50MB
    if not os.path.exists(disk_path):
        print(f"Error: {disk_path} not found. Run make_debug_disk.py first.")
        return

    file_name = os.path.basename(file_path)
    # Convert to 8.3 format
    name_part, ext_part = os.path.splitext(file_name)
    name_part = name_part[:8].upper().ljust(8)
    ext_part = ext_part[1:4].upper().ljust(3)
    fat_name = (name_part + ext_part).encode('ascii')

    with open(file_path, 'rb') as f:
        file_data = f.read()
    
    file_size = len(file_data)

    with open(disk_path, 'r+b') as f:
        # Read BPB from offset
        f.seek(disk_offset + 11)
        bytes_per_sector = struct.unpack('<H', f.read(2))[0]
        sectors_per_cluster = struct.unpack('<B', f.read(1))[0]
        reserved_sectors = struct.unpack('<H', f.read(2))[0]
        f.seek(disk_offset + 16)
        fat_count = struct.unpack('<B', f.read(1))[0]
        f.seek(disk_offset + 17)
        root_entries = struct.unpack('<H', f.read(2))[0]
        f.seek(disk_offset + 22)
        sectors_per_fat = struct.unpack('<H', f.read(2))[0]

        cluster_size = bytes_per_sector * sectors_per_cluster
        fat_start = disk_offset + (reserved_sectors * bytes_per_sector)
        root_start = fat_start + (fat_count * sectors_per_fat * bytes_per_sector)
        data_start = root_start + (root_entries * 32)

        # 1. Scan root directory
        f.seek(root_start)
        entry_offset = -1
        file_exists = False
        
        dir_data = f.read(root_entries * 32)
        for i in range(root_entries):
            entry = dir_data[i*32 : (i+1)*32]
            if entry[0] == 0x00:
                if entry_offset == -1:
                    entry_offset = root_start + i * 32
            elif entry[0] == 0xE5:
                if entry_offset == -1:
                    entry_offset = root_start + i * 32
            else:
                existing_name = entry[0:11]
                if existing_name == fat_name:
                    file_exists = True
                    break

        if file_exists:
            print(f"Error: {file_name} already exists on disk. Skipping upload.")
            return

        if entry_offset == -1:
            print("Error: No free root directory entries.")
            return

        # 2. Find free clusters
        f.seek(fat_start)
        fat_raw = f.read(sectors_per_fat * bytes_per_sector)
        num_entries = len(fat_raw) // 2
        fat_data = list(struct.unpack(f'<{num_entries}H', fat_raw))
        
        free_clusters = []
        needed_clusters = (file_size + cluster_size - 1) // cluster_size
        for i in range(2, len(fat_data)):
            if fat_data[i] == 0x0000:
                free_clusters.append(i)
                if len(free_clusters) == needed_clusters:
                    break
        
        if len(free_clusters) < needed_clusters:
            print("Error: Not enough free space.")
            return

        # 3. Write file data
        for i, cluster in enumerate(free_clusters):
            offset = data_start + (cluster - 2) * cluster_size
            f.seek(offset)
            chunk = file_data[i * cluster_size : (i + 1) * cluster_size]
            f.write(chunk)
        
        # 4. Update FAT
        for i in range(len(free_clusters)):
            cluster = free_clusters[i]
            fat_data[cluster] = 0xFFFF if i == len(free_clusters) - 1 else free_clusters[i+1]
        
        new_fat_bin = struct.pack(f'<{len(fat_data)}H', *fat_data)
        for i in range(fat_count):
            f.seek(fat_start + i * sectors_per_fat * bytes_per_sector)
            f.write(new_fat_bin)

        # 5. Create Directory Entry
        f.seek(entry_offset)
        entry_struct = struct.pack('<11sBBBHHHHHHHI', 
            fat_name, 0x20, 0, 0, 0, 0, 0, 0, 0, 0, 
            free_clusters[0], file_size
        )
        f.write(entry_struct)

        print(f"Successfully uploaded {file_name} to {disk_path} as {name_part.strip()}.{ext_part.strip()}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python upload_pdf.py <file_to_upload>")
    else:
        upload_to_pure_data(sys.argv[1])
