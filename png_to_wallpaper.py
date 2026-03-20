from PIL import Image
import struct, sys, os

input_file = sys.argv[1] if len(sys.argv) > 1 else "wallpaper.png"
output_h = sys.argv[2] if len(sys.argv) > 2 else "src/fs/wallpaper_data.h"
max_w, max_h = 320, 240

img = Image.open(input_file).convert("RGB")
ratio = min(max_w / img.width, max_h / img.height)
new_w = int(img.width * ratio)
new_h = int(img.height * ratio)
img = img.resize((new_w, new_h), Image.LANCZOS)

print(f"Resized to {new_w}x{new_h}")

row_pad = (4 - (new_w * 3) % 4) % 4
pixel_size = (new_w * 3 + row_pad) * new_h
file_size = 54 + pixel_size

bmp = bytearray()
bmp += struct.pack('<2sIHHI', b'BM', file_size, 0, 0, 54)
bmp += struct.pack('<IiiHHIIiiII', 40, new_w, new_h, 1, 24, 0, pixel_size, 2835, 2835, 0, 0)

for y in range(new_h - 1, -1, -1):
    for x in range(new_w):
        r, g, b = img.getpixel((x, y))
        bmp += struct.pack('BBB', b, g, r)
    bmp += b'\x00' * row_pad

os.makedirs(os.path.dirname(output_h), exist_ok=True)
with open(output_h, 'wb') as f:
    f.write(bmp)

print(f"Written raw {len(bmp)} bytes to {output_h}")
