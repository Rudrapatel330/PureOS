"""
Create a 10MB FAT12-formatted hard disk image for PureOS persistent storage.
Only creates the image if it doesn't already exist (to preserve user data).
"""
import struct
import os
import sys

DISK_SIZE = 10 * 1024 * 1024  # 10MB
SECTOR_SIZE = 512
SECTORS_PER_CLUSTER = 8  # 4KB clusters
RESERVED_SECTORS = 1
FAT_COUNT = 2
ROOT_DIR_ENTRIES = 512  # 512 entries = 16KB = 32 sectors
TOTAL_SECTORS = DISK_SIZE // SECTOR_SIZE

# Calculate layout
ROOT_DIR_SECTORS = (ROOT_DIR_ENTRIES * 32 + SECTOR_SIZE - 1) // SECTOR_SIZE
DATA_SECTORS = TOTAL_SECTORS - RESERVED_SECTORS - ROOT_DIR_SECTORS
TOTAL_CLUSTERS = DATA_SECTORS // SECTORS_PER_CLUSTER
# FAT12: each entry is 1.5 bytes
FAT_SIZE_BYTES = (TOTAL_CLUSTERS * 3 + 1) // 2
SECTORS_PER_FAT = (FAT_SIZE_BYTES + SECTOR_SIZE - 1) // SECTOR_SIZE

# Recalculate with FATs included
DATA_START = RESERVED_SECTORS + (FAT_COUNT * SECTORS_PER_FAT) + ROOT_DIR_SECTORS

OUTPUT = "data.img"

def create_fat12_image():
    if os.path.exists(OUTPUT):
        print(f"{OUTPUT} already exists - preserving user data (not overwriting)")
        return

    print(f"Creating {OUTPUT} ({DISK_SIZE // 1024}KB FAT12 data disk)...")
    disk = bytearray(DISK_SIZE)

    # === Boot Sector / BPB ===
    bpb = bytearray(SECTOR_SIZE)
    bpb[0:3] = b'\xEB\x3C\x90'  # JMP short + NOP
    bpb[3:11] = b'PUREOS  '     # OEM Name
    struct.pack_into('<H', bpb, 11, SECTOR_SIZE)           # Bytes per sector
    bpb[13] = SECTORS_PER_CLUSTER                           # Sectors per cluster
    struct.pack_into('<H', bpb, 14, RESERVED_SECTORS)       # Reserved sectors
    bpb[16] = FAT_COUNT                                     # Number of FATs
    struct.pack_into('<H', bpb, 17, ROOT_DIR_ENTRIES)       # Root dir entries
    struct.pack_into('<H', bpb, 19, min(TOTAL_SECTORS, 0xFFFF))  # Total sectors (16-bit)
    bpb[21] = 0xF8                                          # Media descriptor (fixed disk)
    struct.pack_into('<H', bpb, 22, SECTORS_PER_FAT)        # Sectors per FAT
    struct.pack_into('<H', bpb, 24, 63)                     # Sectors per track
    struct.pack_into('<H', bpb, 26, 16)                     # Number of heads
    struct.pack_into('<I', bpb, 28, 0)                      # Hidden sectors
    struct.pack_into('<I', bpb, 32, TOTAL_SECTORS)          # Total sectors (32-bit)

    # Extended BPB
    bpb[36] = 0x80          # Drive number (hard disk)
    bpb[37] = 0             # Reserved
    bpb[38] = 0x29          # Boot signature
    struct.pack_into('<I', bpb, 39, 0x12345678)  # Volume serial
    bpb[43:54] = b'PUREOS DATA'                   # Volume label
    bpb[54:62] = b'FAT12   '                      # FS Type

    # Boot signature
    bpb[510] = 0x55
    bpb[511] = 0xAA

    disk[0:SECTOR_SIZE] = bpb

    # === FAT Tables ===
    fat_start = RESERVED_SECTORS * SECTOR_SIZE
    for f in range(FAT_COUNT):
        offset = fat_start + f * SECTORS_PER_FAT * SECTOR_SIZE
        # First two entries: media descriptor + 0xFFF
        disk[offset] = 0xF8
        disk[offset + 1] = 0xFF
        disk[offset + 2] = 0xFF

    # === Root Directory ===
    root_start = (RESERVED_SECTORS + FAT_COUNT * SECTORS_PER_FAT) * SECTOR_SIZE

    # Volume Label entry
    vol_label = bytearray(32)
    vol_label[0:11] = b'PUREOS DATA'
    vol_label[11] = 0x08  # Volume label attribute
    disk[root_start:root_start + 32] = vol_label

    # Pre-create DOCS folder
    def make_dir_entry(name_8_3, is_dir=False, cluster=0):
        entry = bytearray(32)
        padded = name_8_3.ljust(11).encode('ascii')[:11]
        entry[0:11] = padded
        entry[11] = 0x10 if is_dir else 0x20  # Directory or Archive
        struct.pack_into('<H', entry, 26, cluster)  # First cluster
        struct.pack_into('<I', entry, 28, 0)  # Size (0 for dirs)
        return entry

    # Allocate clusters for pre-built directories
    # Cluster 2 = DOCS, Cluster 3 = PROGRAMS, Cluster 4 = SYSTEM
    dirs = [
        ("DOCS       ", 2),
        ("PROGRAMS   ", 3),
        ("SYSTEM     ", 4),
    ]

    for i, (name, cluster) in enumerate(dirs):
        entry = make_dir_entry(name, is_dir=True, cluster=cluster)
        offset = root_start + (i + 1) * 32  # +1 to skip volume label
        disk[offset:offset + 32] = entry

        # Mark cluster as used in FAT (end-of-chain marker)
        fat_byte_offset = cluster + (cluster // 2)
        for f in range(FAT_COUNT):
            fat_off = fat_start + f * SECTORS_PER_FAT * SECTOR_SIZE + fat_byte_offset
            if cluster & 1:
                disk[fat_off] = (disk[fat_off] & 0x0F) | 0xF0
                disk[fat_off + 1] = 0xFF
            else:
                disk[fat_off] = 0xF8
                disk[fat_off + 1] = (disk[fat_off + 1] & 0xF0) | 0x0F

        # Initialize directory cluster with . and .. entries
        cluster_offset = (DATA_START + (cluster - 2) * SECTORS_PER_CLUSTER) * SECTOR_SIZE
        # . entry
        dot = make_dir_entry(".          ", is_dir=True, cluster=cluster)
        disk[cluster_offset:cluster_offset + 32] = dot
        # .. entry (parent = root = 0)
        dotdot = make_dir_entry("..         ", is_dir=True, cluster=0)
        disk[cluster_offset + 32:cluster_offset + 64] = dotdot

    # Write image
    with open(OUTPUT, 'wb') as f:
        f.write(disk)

    print(f"Created {OUTPUT} successfully!")
    print(f"  Total Size: {DISK_SIZE // 1024}KB")
    print(f"  Sectors per FAT: {SECTORS_PER_FAT}")
    print(f"  Root Dir Sectors: {ROOT_DIR_SECTORS}")
    print(f"  Data Start Sector: {DATA_START}")
    print(f"  Pre-created folders: DOCS, PROGRAMS, SYSTEM")

if __name__ == '__main__':
    create_fat12_image()
