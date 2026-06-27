import struct
import os

def create_debug_disk():
    # 1. Gather components
    boot_data = b''
    if os.path.exists('build/boot_sect.bin'):
        with open('build/boot_sect.bin', 'rb') as f:
            boot_data = f.read()
    
    stage2_data = b''
    if os.path.exists('build/stage2.bin'):
        with open('build/stage2.bin', 'rb') as f:
            stage2_data = f.read()
            
    kernel_data = b''
    if os.path.exists('build/kernel.bin'):
        with open('build/kernel.bin', 'rb') as f:
            kernel_data = f.read()
            print(f"Kernel Size: {len(kernel_data)} bytes")
    
    if len(kernel_data) == 0:
        print("ERROR: build/kernel.bin not found or empty! Aborting.")
        import sys
        sys.exit(1)

    # 2. Calculate needed size (Padded to 128MB to include data partition)
    # 50MB for kernel/system, 78MB for data.
    min_size = 128 * 1024 * 1024 
    required_size = 2048 + len(kernel_data)
    
    total_size = max(min_size, required_size)
    
    # 3. Align to 512 bytes for VirtualBox compatibility
    if total_size % 512 != 0:
        total_size = (total_size + 511) // 512 * 512
        
    print(f"Total Disk Image Size: {total_size} bytes")
    disk = bytearray(total_size)
    
    # 4. Fill disk
    if boot_data:
        disk[0:len(boot_data)] = boot_data
    
    # Boot signature
    disk[510:512] = b'\x55\xAA'
    
    if stage2_data:
        disk[512:512+len(stage2_data)] = stage2_data
        print(f"Stage 2 Size: {len(stage2_data)} bytes")

    # 4.5 Inject kernel sector count for Stage 2 (last 4 bytes of Sector 3)
    # The bootloader is loaded at 0x7E00, so this will be at 0x83FC.
    kernel_sectors = (len(kernel_data) + 511) // 512
    print(f"Kernel Sectors: {kernel_sectors}")
    disk[2044:2048] = kernel_sectors.to_bytes(4, byteorder='little')
        
    if kernel_data:
        disk[2048:2048+len(kernel_data)] = kernel_data

    # 4.6 Merge Data Disk if exists (at 50MB offset = sector 102400)
    data_offset = 50 * 1024 * 1024
    if os.path.exists('pure_data.img'):
        with open('pure_data.img', 'rb') as f:
            data_disk = f.read()
            if len(data_disk) > (total_size - data_offset):
                print("Warning: data disk too large, truncating")
                data_disk = data_disk[:(total_size - data_offset)]
            disk[data_offset:data_offset+len(data_disk)] = data_disk
            print(f"Merged pure_data.img at offset {data_offset}")

    # 5. Write to file
    with open('pureos.img', 'wb') as f:
        f.write(disk)
    print("Created pureos.img")

    # 6. Upload Icons
    try:
        from bulk_upload_icons import main as upload_icons
        print("Uploading icons...")
        upload_icons()
    except ImportError:
        print("Warning: bulk_upload_icons.py not found, skipping icon upload")
    except Exception as e:
        print(f"Error uploading icons: {e}")

    # 7. Upload Songs
    try:
        from bulk_upload_songs import main as upload_songs
        print("Uploading songs...")
        upload_songs()
    except ImportError:
        print("Warning: bulk_upload_songs.py not found, skipping song upload")
    except Exception as e:
        print(f"Error uploading songs: {e}")

    # 8. Upload PDFs
    print("Uploading PDFs...")
    try:
        import subprocess
        if os.path.exists("pdftest.pdf"):
            subprocess.run(["python", "upload_pdf.py", "pdftest.pdf"], check=False)
        if os.path.exists("Building OS Networking and Search.pdf"):
            subprocess.run(["python", "upload_pdf.py", "Building OS Networking and Search.pdf"], check=False)
    except Exception as e:
        print(f"Error uploading PDFs: {e}")

    # 9. Upload Hill Climb Assets
    print("Uploading Hill Climb Assets...")
    try:
        if os.path.exists("car.png"): subprocess.run(["python", "upload_pdf.py", "car.png"], check=False)
        if os.path.exists("tire.png"): subprocess.run(["python", "upload_pdf.py", "tire.png"], check=False)
        if os.path.exists("ground.png"): subprocess.run(["python", "upload_pdf.py", "ground.png"], check=False)
        if os.path.exists("surf.png"): subprocess.run(["python", "upload_pdf.py", "surf.png"], check=False)
        if os.path.exists("coin5.png"): subprocess.run(["python", "upload_pdf.py", "coin5.png"], check=False)
        if os.path.exists("coin25.png"): subprocess.run(["python", "upload_pdf.py", "coin25.png"], check=False)
        if os.path.exists("coin100.png"): subprocess.run(["python", "upload_pdf.py", "coin100.png"], check=False)
        if os.path.exists("coin500.png"): subprocess.run(["python", "upload_pdf.py", "coin500.png"], check=False)
    except Exception as e:
        print(f"Error uploading Hill Climb assets: {e}")

if __name__ == '__main__':
    create_debug_disk()
