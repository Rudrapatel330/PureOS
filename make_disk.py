# make_disk.py
import sys
import os

def create_disk():
    print("Creating PureOS disk image...")
    
    try:
        # Read files
        with open('build/boot_sect.bin', 'rb') as f:
            boot = f.read()
        
        with open('build/stage2.bin', 'rb') as f:
            stage2 = f.read()
        
        with open('build/kernel.bin', 'rb') as f:
            kernel = f.read()
        
        # Create disk image
        with open('build/pureos.img', 'wb') as f:
            # 1. Boot sector (sector 0)
            f.write(boot)
            print(f"  Boot sector: {len(boot)} bytes")
            
            # 2. Stage2 (sectors 1-8)
            f.write(stage2)
            stage2_sectors = (len(stage2) + 511) // 512
            print(f"  Stage2: {len(stage2)} bytes ({stage2_sectors} sectors)")
            
            # 3. Pad to sector 100 (leave space)
            current_pos = len(boot) + len(stage2)
            target_pos = 100 * 512
            if current_pos < target_pos:
                f.write(b'\x00' * (target_pos - current_pos))
                print(f"  Padding: {target_pos - current_pos} bytes")
            
            # 4. Write kernel at sector 100
            kernel_pos = f.tell()
            f.write(kernel)
            kernel_sectors = (len(kernel) + 511) // 512
            print(f"  Kernel at sector {kernel_pos//512}: {len(kernel)} bytes ({kernel_sectors} sectors)")
            
            # 5. Pad to 1.44MB
            f.seek(1474560 - 1)
            f.write(b'\x00')
            
            total_size = f.tell()
            print(f"\nTotal: {total_size} bytes ({total_size/1024/1024:.2f} MB)")
            
            # Print sector map
            print("\nSector Map:")
            print(f"  0: Boot sector")
            print(f"  1-{stage2_sectors}: Stage2")
            print(f"  100-{100 + kernel_sectors - 1}: Kernel")
            
            return kernel_pos // 512
    except Exception as e:
        print(f"Error: {e}")
        return -1

if __name__ == '__main__':
    kernel_sector = create_disk()
    if kernel_sector != -1:
        print(f"\nKernel starts at sector: {kernel_sector}")
        try:
            with open('build/kernel_sector.txt', 'w') as f:
                f.write(str(kernel_sector))
        except:
            pass
    exit(0 if kernel_sector != -1 else 1)
