import os
import struct

def create_fat_data_disk():
    # 75MB Disk - fits in the 78MB data partition slot (128MB total - 50MB kernel offset)
    disk_size = 75 * 1024 * 1024  # 78643200 bytes
    sector_size = 512
    sectors_per_cluster = 8  # 4KB clusters
    reserved_sectors = 1
    fat_count = 2
    root_entries = 512  # 512 entries = 16KB = 32 sectors
    total_sectors = disk_size // sector_size

    # Calculate layout dynamically
    root_dir_sectors = (root_entries * 32 + sector_size - 1) // sector_size

    # For FAT16: each entry is 2 bytes
    # We need to calculate sectors_per_fat based on total data clusters
    # Formula: total_clusters = (total_sectors - reserved - root_dir_sectors - 2*spf) / spc
    # FAT size must cover all clusters: spf >= (total_clusters * 2) / 512
    # Solve iteratively:
    data_sectors_approx = total_sectors - reserved_sectors - root_dir_sectors
    total_clusters_approx = data_sectors_approx // sectors_per_cluster
    fat_size_bytes = total_clusters_approx * 2
    sectors_per_fat = (fat_size_bytes + sector_size - 1) // sector_size

    # Recalculate with FATs included
    data_start_sector = reserved_sectors + (fat_count * sectors_per_fat) + root_dir_sectors

    image = bytearray(disk_size)

    # === Boot Sector / BPB ===
    image[0:3] = b'\xeb\x3c\x90'       # JMP short + NOP
    image[3:11] = b'MSDOS5.0'           # OEM Name
    struct.pack_into('<H', image, 11, sector_size)        # Bytes per sector
    image[13] = sectors_per_cluster                        # Sectors per cluster
    struct.pack_into('<H', image, 14, reserved_sectors)    # Reserved sectors
    image[16] = fat_count                                  # Number of FATs
    struct.pack_into('<H', image, 17, root_entries)        # Root dir entries
    struct.pack_into('<H', image, 19, 0)                   # Total sectors (16-bit) - 0 means use 32-bit
    image[21] = 0xf8                                       # Media descriptor (fixed disk)
    struct.pack_into('<H', image, 22, sectors_per_fat)     # Sectors per FAT
    struct.pack_into('<H', image, 24, 63)                  # Sectors per track
    struct.pack_into('<H', image, 26, 16)                  # Number of heads
    struct.pack_into('<I', image, 28, 0)                   # Hidden sectors
    struct.pack_into('<I', image, 32, total_sectors)       # Total sectors (32-bit)

    # Extended BPB
    image[36] = 0x80          # Drive number (hard disk)
    image[37] = 0             # Reserved
    image[38] = 0x29          # Boot signature
    struct.pack_into('<I', image, 39, 0x12345678)  # Volume serial
    image[43:54] = b'PURE_DATA  '                  # Volume label
    image[54:62] = b'FAT16   '                     # FS Type

    # Boot signature
    image[510] = 0x55
    image[511] = 0xaa

    # === FAT Tables ===
    # FAT16 Entry 0: F8FF (media descriptor), Entry 1: FFFF (end-of-chain)
    fat1_offset = reserved_sectors * sector_size
    struct.pack_into('<H', image, fat1_offset, 0xfff8)
    struct.pack_into('<H', image, fat1_offset + 2, 0xffff)

    fat2_offset = fat1_offset + sectors_per_fat * sector_size
    struct.pack_into('<H', image, fat2_offset, 0xfff8)
    struct.pack_into('<H', image, fat2_offset + 2, 0xffff)

    # === Root Directory ===
    root_start = (reserved_sectors + fat_count * sectors_per_fat) * sector_size

    # Volume Label entry
    image[root_start:root_start + 11] = b'PURE_DATA  '
    image[root_start + 11] = 0x08  # Volume label attribute

    # Pre-create directories: DOCS, PROGRAMS, SYSTEM
    def make_dir_entry(name_11, cluster):
        entry = bytearray(32)
        entry[0:11] = name_11.encode('ascii')[:11]
        entry[11] = 0x10  # Directory attribute
        struct.pack_into('<H', entry, 26, cluster)
        struct.pack_into('<I', entry, 28, 0)
        return entry

    dirs = [
        ("DOCS       ", 2),
        ("PROGRAMS   ", 3),
        ("SYSTEM     ", 4),
    ]

    for i, (name, cluster) in enumerate(dirs):
        entry = make_dir_entry(name, cluster)
        offset = root_start + (i + 1) * 32
        image[offset:offset + 32] = entry

        # Mark cluster as used in both FATs (end-of-chain = 0xFFFF)
        for fat_idx in range(fat_count):
            fat_base = fat1_offset + fat_idx * sectors_per_fat * sector_size
            struct.pack_into('<H', image, fat_base + cluster * 2, 0xffff)

        # Initialize directory cluster with . and .. entries
        cluster_offset = (data_start_sector + (cluster - 2) * sectors_per_cluster) * sector_size

        # . entry
        dot = bytearray(32)
        dot[0:11] = b'.          '
        dot[11] = 0x10
        struct.pack_into('<H', dot, 26, cluster)
        image[cluster_offset:cluster_offset + 32] = dot

        # .. entry (parent = root = 0)
        dotdot = bytearray(32)
        dotdot[0:11] = b'..         '
        dotdot[11] = 0x10
        struct.pack_into('<H', dotdot, 26, 0)
        image[cluster_offset + 32:cluster_offset + 64] = dotdot

    with open('pure_data.img', 'wb') as f:
        f.write(image)

    print(f"Created pure_data.img ({len(image)} bytes = {len(image) // (1024*1024)}MB)")
    print(f"  Total Sectors: {total_sectors}")
    print(f"  Sectors per FAT: {sectors_per_fat}")
    print(f"  Root Dir Sectors: {root_dir_sectors}")
    print(f"  Data Start Sector: {data_start_sector}")
    print(f"  Pre-created folders: DOCS, PROGRAMS, SYSTEM")

if __name__ == '__main__':
    if os.path.exists('pure_data.img'):
        print("pure_data.img already exists, skipping creation to preserve persistent data.")
    else:
        create_fat_data_disk()
