@echo off
echo ===== CHECKING DISK SECTOR 100 =====

python -c "
import os

img_path = 'build/pureos.img'
if not os.path.exists(img_path):
    print('✗ Image not found:', img_path)
    exit(1)

size = os.path.getsize(img_path)
print(f'Image size: {size} bytes')
print(f'Total sectors: {size // 512}')

with open(img_path, 'rb') as f:
    # Check sector 100 (100 * 512 = 51200 bytes)
    f.seek(51200)
    sector100 = f.read(512)
    
    # Count non-zero bytes
    non_zero = sum(1 for b in sector100 if b != 0)
    print(f'\nSector 100:')
    print(f'  Non-zero bytes: {non_zero}/512')
    print(f'  First 16 bytes: {sector100[:16].hex()}')
    
    if non_zero > 0:
        print(f'  ✓ Sector 100 HAS DATA!')
    else:
        print(f'  ✗ Sector 100 is ALL ZEROS!')
    
    # Check where kernel actually is
    print(f'\nSearching for kernel...')
    try:
        with open('build/kernel.bin', 'rb') as k:
            kernel_start = k.read(16)
        
        # Search whole image
        f.seek(0)
        img_data = f.read()
        pos = img_data.find(kernel_start)
        
        if pos != -1:
            sector = pos // 512
            print(f'✓ Found kernel at byte {pos} (sector {sector})')
            print(f'  That is {sector} sectors from start')
        else:
            print('✗ Kernel NOT FOUND in image!')
            
            # List first few non-zero sectors
            print('\nFirst 5 non-zero sectors:')
            for s in range(0, min(50, size//512)):
                f.seek(s * 512)
                data = f.read(512)
                if any(b != 0 for b in data):
                    print(f'  Sector {s}: {data[:8].hex()}...')
    except FileNotFoundError:
        print('Warning: build/kernel.bin missing, skipping search')
"
pause
