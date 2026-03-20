import struct

try:
    with open('build/pureos.img', 'wb') as f:
        # Boot sector (from earlier build)
        with open('build/boot_sect.bin', 'rb') as b:
            f.write(b.read())
        
        # Stage2 (4KB = 8 sectors)
        with open('build/stage2.bin', 'rb') as s:
            f.write(s.read())
        
        # Pad to sector 13 (boot sector + 12 sectors for stage2 => next is sector 13)
        # Sector 0 = boot (512)
        # Sector 1 = stage2 start. 
        # We want kernel at Sector 13 (Offset 13*512 = 6656).
        # Currently written: 512 (boot) + 4096 (stage2) = 4608 bytes.
        # Need to pad to 6656.
        current = f.tell()
        needed = 13 * 512
        if current < needed:
            f.write(b'\x00' * (needed - current))
        
        # Write kernel at sector 13
        with open('build/kernel.bin', 'rb') as k:
            f.write(k.read())
        
        # Pad to 1.44MB
        f.seek(1474560-1)
        f.write(b'\x00')
        
    print('Image created:')
    print('  Sector 0: Boot sector')
    print('  Sectors 1-8: Stage2 (4KB)')
    print('  Sector 13+: Kernel')

except Exception as e:
    print(f"Error creating image: {e}")
    exit(1)
