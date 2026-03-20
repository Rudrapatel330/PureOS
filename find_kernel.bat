@echo off
echo ===== FINDING KERNEL ON DISK =====

echo 1. Checking kernel.bin size and content...
python -c "
import os
try:
    size = os.path.getsize('build/kernel.bin')
    print(f'kernel.bin size: {size} bytes ({size/512:.1f} sectors)')

    with open('build/kernel.bin', 'rb') as f:
        first_16 = f.read(16)
        print(f'First 16 bytes: {first_16.hex()}')
        print(f'As text: {first_16}')
        
        # Check for common patterns
        if first_16.startswith(b'\x7fELF'):
            print('✓ ELF header detected')
        elif b'kernel_main' in first_16:
            print('✓ Kernel text detected')
        else:
            print('? Unknown content')
except FileNotFoundError:
    print('Error: build/kernel.bin not found!')
"

echo.
echo 2. Searching pureos.img for kernel content...
python -c "
try:
    with open('build/pureos.img', 'rb') as f:
        img_data = f.read()
        
        # Look for kernel.bin content
        try:
            with open('build/kernel.bin', 'rb') as k:
                kernel_data = k.read(512)  # Just first sector
                
            # Search for this pattern in image
            pos = img_data.find(kernel_data[:32])  # Search for first 32 bytes
            if pos != -1:
                sector = pos // 512
                print(f'✓ Found kernel at byte {pos} (sector {sector})')
                print(f'  That is sector {sector} on disk')
                
                # Show what's around it
                print(f'  Bytes at sector {sector}: {img_data[pos:pos+32].hex()}')
            else:
                print('✗ Kernel NOT FOUND in image!')
        except FileNotFoundError:
            print('Skipping search: build/kernel.bin missing')
            
        # Also check sector 13 specifically
        sector13 = img_data[13*512:13*512+32]
        print(f'  Sector 13 content: {sector13.hex()}')
except FileNotFoundError:
    print('Error: build/pureos.img not found!')
"

echo.
echo 3. Quick fix: Write kernel to KNOWN location
python -c "
try:
    with open('build/pureos.img', 'r+b') as f:
        # Write kernel to sector 100 (far enough to avoid conflicts)
        f.seek(100 * 512)
        with open('build/kernel.bin', 'rb') as k:
            f.write(k.read())
        
        print('✓ Kernel written to sector 100')
        
        # Also backup sector 13
        f.seek(13 * 512)
        sector13 = f.read(32)
        print(f'  Sector 13 backup: {sector13.hex()}')
except FileNotFoundError:
    print('Skipping write: Files missing')
"

pause
