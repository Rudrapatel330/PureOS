import os
import struct
import sys
from PIL import Image

def upload_to_pure_data(file_path, target_name=None):
    if not os.path.exists(file_path) and not isinstance(file_path, bytes):
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

    if isinstance(file_path, bytes):
        file_data = file_path
    else:
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
                    entry_offset = root_start + i * 32
                    break

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
    icons_dir = 'icons'
    if os.path.exists(icons_dir):
        for filename in os.listdir(icons_dir):
            if filename.lower().endswith('.png'):
                full_path = os.path.join(icons_dir, filename)
                try:
                    img = Image.open(full_path).convert('RGBA')
                    
                    alpha = img.split()[-1]
                    bbox = alpha.getbbox()
                    if bbox:
                        img = img.crop(bbox)
                    
                    safe_size = 416
                    img.thumbnail((safe_size, safe_size), Image.Resampling.LANCZOS)
                    
                    target_size = 512
                    new_img = Image.new('RGBA', (target_size, target_size), (0, 0, 0, 0))
                    
                    x = (target_size - img.width) // 2
                    y = (target_size - img.height) // 2
                    
                    new_img.paste(img, (x, y), img)
                    
                    temp_path = os.path.join(icons_dir, "temp_" + filename)
                    new_img.save(temp_path, "PNG")
                    
                    upload_to_pure_data(temp_path, filename)
                    os.remove(temp_path)
                except Exception as e:
                    print(f"Failed to process {filename}: {e}")
                    upload_to_pure_data(full_path)
            
    wallpaper_dir = 'wallpaper'
    if os.path.exists(wallpaper_dir):
        wallpapers = [f for f in os.listdir(wallpaper_dir) if f.lower().endswith(('.jpg', '.png'))]
        wallpapers.sort() # Ensure consistent wall1, wall2...
        
        for i, filename in enumerate(wallpapers):
            full_path = os.path.join(wallpaper_dir, filename)
            target_name = f"WALL{i+1}.JPG"
            upload_to_pure_data(full_path, target_name)
            
            # PRE-GENERATE THUMBNAIL (Windows/macOS Strategy)
            print(f"Optimizing {filename} for instant load...")
            try:
                img = Image.open(full_path).convert('RGBA')
                img = img.resize((480, 300), Image.Resampling.LANCZOS)
                
                # Convert to ARGB raw format for the OS
                raw_data = bytearray()
                pixels = img.load()
                for y in range(300):
                    for x in range(480):
                        r, g, b, a = pixels[x, y]
                        # Pack as AARRGGBB
                        raw_data.extend(struct.pack('<I', (a << 24) | (r << 16) | (g << 8) | b))
                
                # Upload as a cache file that the OS will find instantly
                # Use a specific name that the OS's cache logic will hit
                upload_to_pure_data(bytes(raw_data), f"W{i+1}THUMB.RAW")
            except Exception as e:
                print(f"Failed to generate thumb for {filename}: {e}")
                
    if os.path.exists('test.html'):
        upload_to_pure_data('test.html', 'TEST.HTM')

if __name__ == '__main__':
    main()
