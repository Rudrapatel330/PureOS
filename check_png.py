import struct

def get_png_info(file_path):
    with open(file_path, 'rb') as f:
        signature = f.read(8)
        if signature != b'\x89PNG\r\n\x1a\n':
            return "Not a PNG"
        
        # Read IHDR chunk
        f.seek(12)
        chunk_type = f.read(4)
        if chunk_type != b'IHDR':
            return "No IHDR"
        
        width, height = struct.unpack('>II', f.read(8))
        return f"{width}x{height}"

print(get_png_info('src/fs/wallpaper.png'))
