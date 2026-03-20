
import os
import struct

try:
    print('Creating disk image...')
    
    # Read files
    try:
        with open('build/boot.bin', 'rb') as f: boot = f.read()
        with open('build/stage2.bin', 'rb') as f: stage2 = f.read()  
        with open('build/kernel.bin', 'rb') as f: kernel = f.read()
    except FileNotFoundError as e:
        print(f"Error reading files: {e}")
        exit(1)

    print(f'Boot: {len(boot)} bytes')
    print(f'Stage2: {len(stage2)} bytes')
    print(f'Kernel: {len(kernel)} bytes')

    # Create image with exact layout
    with open('build/pureos_final.img', 'wb') as f:
        # Sector 0: Boot
        f.write(boot)
        
        # Sectors 1-12: Stage2 (12 sectors = 6KB)
        f.write(stage2)
        if len(stage2) < 12*512:
            f.write(b'\x00' * (12*512 - len(stage2)))
        
        # Sector 13: Kernel
        kernel_pos = f.tell()
        f.write(kernel)
        
        # Pad to 1.44MB
        f.seek(1474560 - 1)
        f.write(b'\x00')

    print(f'\nDisk layout:')
    print(f'  Sector 0: Boot')
    print(f'  Sectors 1-12: Stage2')
    print(f'  Sector 13: Kernel (starts at byte {kernel_pos})')
    print(f'  Kernel first byte should be: 0x55')

    # Verify
    with open('build/pureos_final.img', 'rb') as f:
        f.seek(13 * 512)
        first_byte = f.read(1)[0]
        print(f'\nVerification:')
        print(f'  Sector 13 first byte: 0x{first_byte:02X}')
        if first_byte == 0x55:
            print('  ✓ CORRECT!')
        else:
            print('  ✗ WRONG!')

except Exception as e:
    print(f"Error: {e}")
    exit(1)
