import os

def check_bin():
    if not os.path.exists('build/kernel.bin'):
        print("build/kernel.bin not found!")
        return

    size = os.path.getsize('build/kernel.bin')
    print(f"Kernel size: {size} bytes")
    
    with open('build/kernel.bin', 'rb') as f:
        data = f.read(64)
        print('Kernel.bin first 64 bytes:')
        hex_str = data.hex()
        for i in range(0, len(hex_str), 32):
            print(f"{i//2:04x}: {hex_str[i:i+32]}")

if __name__ == "__main__":
    check_bin()
