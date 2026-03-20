import glob
import os

def check_kernel_lba():
    # Find the latest pureos image
    files = glob.glob('pureos_*.img')
    if not files:
        print("No pureos_*.img files found.")
        return
        
    latest_file = max(files, key=os.path.getctime)
    print(f"Inspecting {latest_file}...")
    
    with open(latest_file, 'rb') as f:
        f.seek(36 * 512)  # LBA 36
        data = f.read(512)
        print('First 64 bytes of kernel (LBA 36):')
        print(data[:64].hex())
        print()
        print('As ASCII (if text):')
        print(data[:64].decode('ascii', errors='ignore'))
        
        # Check signature
        if data[:4] == b'\x7FELF':
            print("✓ ELF Header detected!")
        else:
            print("? No ELF header found.")

if __name__ == "__main__":
    check_kernel_lba()
