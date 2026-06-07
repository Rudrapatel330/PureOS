import os
import struct
import sys

def upload_to_pure_data(file_path, target_name=None):
    if not os.path.exists(file_path):
        print(f"Error: {file_path} not found.")
        return

    disk_path = 'pureos.img'
    disk_offset = 50 * 1024 * 1024 # 50MB
    if not os.path.exists(disk_path):
        print(f"Error: {disk_path} not found. Run make_debug_disk.py first.")
        return

    real_file_name = target_name if target_name else os.path.basename(file_path)
    
    # Convert to 8.3 format
    name_part, ext_part = os.path.splitext(real_file_name)
    name_part = name_part[:8].upper().ljust(8)
    ext_part = ext_part[1:4].upper().ljust(3)
    fat_name = (name_part + ext_part).encode('ascii')

    with open(file_path, 'rb') as f:
        file_data = f.read()
    
    file_size = len(file_data)

    with open(disk_path, 'r+b') as f:
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

        f.seek(root_start)
        entry_offset = -1
        dir_data = f.read(root_entries * 32)
        for i in range(root_entries):
            entry = dir_data[i*32 : (i+1)*32]
            if entry[0] in [0x00, 0xE5]:
                if entry_offset == -1: entry_offset = root_start + i * 32
            else:
                if entry[0:11] == fat_name:
                    # Overwrite if exists
                    entry_offset = root_start + i * 32
                    break

        if entry_offset == -1:
             print(f"Error: No root directory entries available for {real_file_name}.")
             return

        f.seek(fat_start)
        fat_raw = f.read(sectors_per_fat * bytes_per_sector)
        num_entries = len(fat_raw) // 2
        fat_data = list(struct.unpack(f'<{num_entries}H', fat_raw))
        
        free_clusters = []
        needed_clusters = (file_size + cluster_size - 1) // cluster_size
        for i in range(2, len(fat_data)):
            if fat_data[i] == 0x0000:
                free_clusters.append(i)
                if len(free_clusters) == needed_clusters: break
        
        if len(free_clusters) < needed_clusters:
            print(f"Error: Not enough space for {real_file_name}.")
            return

        for i, cluster in enumerate(free_clusters):
            offset = data_start + (cluster - 2) * cluster_size
            f.seek(offset)
            chunk = file_data[i * cluster_size : (i + 1) * cluster_size]
            if len(chunk) < cluster_size: chunk += b'\x00' * (cluster_size - len(chunk))
            f.write(chunk)
        
        for i in range(len(free_clusters)):
            cluster = free_clusters[i]
            fat_data[cluster] = 0xFFFF if i == len(free_clusters) - 1 else free_clusters[i+1]
        
        new_fat_bin = struct.pack(f'<{len(fat_data)}H', *fat_data)
        for i in range(fat_count):
            f.seek(fat_start + i * sectors_per_fat * bytes_per_sector)
            f.write(new_fat_bin)

        f.seek(entry_offset)
        entry_struct = struct.pack('<11sBBBHHHHHHHI', 
            fat_name, 0x20, 0, 0, 0, 0, 0, 0, 0, 0, free_clusters[0], file_size
        )
        f.write(entry_struct)
        print(f"Uploaded {real_file_name} successfully.")

def main():
    songs_dir = 'songs'
    covers_dir = os.path.join('songs', 'covers')
    
    if os.path.exists(songs_dir):
        for filename in os.listdir(songs_dir):
            if filename.lower().endswith('.mp3'):
                full_path = os.path.join(songs_dir, filename)
                upload_to_pure_data(full_path, filename)
                
                # Check for corresponding cover
                base_name = os.path.splitext(filename)[0]
                cover_name = base_name + '.png'
                cover_path = os.path.join(covers_dir, cover_name)
                if not os.path.exists(cover_path):
                    # Try uppercase or other variations if needed, or fallback to root covers dir
                    alt_covers_dir = 'covers'
                    cover_path = os.path.join(alt_covers_dir, cover_name)
                
                if os.path.exists(cover_path):
                    upload_to_pure_data(cover_path, cover_name)

if __name__ == '__main__':
    main()
