
import os
import sys

img_path = 'build/pureos.img'
kernel_path = 'build/kernel.bin'

print('1. Checking files...')
print(f'  Image exists: {os.path.exists(img_path)}')
print(f'  Kernel.bin exists: {os.path.exists(kernel_path)}')

if os.path.exists(kernel_path):
    with open(kernel_path, 'rb') as f:
        kernel_data = f.read()
        print(f'  Kernel size: {len(kernel_data)} bytes')
        print(f'  Kernel first 8 bytes: {kernel_data[:8].hex()}')
        
        # Kernel should start with 0xB8 (MOV instruction)
        if len(kernel_data) > 0:
            print(f'  First byte: 0x{kernel_data[0]:02X}')
            if kernel_data[0] == 0xB8:
                print('  ✓ Starts with 0xB8 (MOV instruction)')

if os.path.exists(img_path):
    with open(img_path, 'rb') as f:
        img_size = os.path.getsize(img_path)
        print(f'\n2. Image size: {img_size} bytes ({img_size//512} sectors)')
        
        # Search for kernel in image
        print('\n3. Searching for kernel in image...')
        
        if os.path.exists(kernel_path):
            with open(kernel_path, 'rb') as k:
                kernel_start = k.read(32)  # First 32 bytes of kernel
                
            # Search whole image
            f.seek(0)
            img_content = f.read()
            
            # Try exact match first
            pos = img_content.find(kernel_start)
            if pos != -1:
                sector = pos // 512
                print(f'  ✓ Found EXACT match at byte {pos} (sector {sector})')
                
                # Show surrounding sectors
                for s in range(max(0, sector-2), min(sector+3, img_size//512)):
                    f.seek(s * 512)
                    data = f.read(16)
                    non_zero = sum(1 for b in data if b != 0)
                    marker = '← KERNEL HERE' if s == sector else ''
                    print(f'    Sector {s:3d}: {data.hex()} {marker}')
            else:
                print('  ✗ No exact match found')
                
                # Search for any non-zero sectors
                print('\n4. Listing first non-zero sectors:')
                count = 0
                for s in range(0, min(100, img_size//512)):
                    f.seek(s * 512)
                    data = f.read(512)
                    non_zero = sum(1 for b in data if b != 0)
                    if non_zero > 0:
                        print(f'    Sector {s:3d}: {data[:16].hex()}... (has {non_zero} non-zero bytes)')
                        count += 1
                        if count >= 10:
                            break
        
        # Also check specific sectors
        print('\n5. Checking specific sectors:')
        check_sectors = [0, 1, 12, 13, 50, 100]
        for s in check_sectors:
            f.seek(s * 512)
            data = f.read(16)
            non_zero = sum(1 for b in data if b != 0)
            status = 'HAS DATA' if non_zero > 0 else 'empty'
            print(f'    Sector {s:3d}: {data.hex()} ({status})')
