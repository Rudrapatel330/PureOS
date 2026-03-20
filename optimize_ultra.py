from PIL import Image
import os

def optimize_hard(input_path, output_path):
    try:
        with Image.open(input_path) as img:
            # Resize
            img = img.resize((1600, 900), Image.Resampling.LANCZOS)
            # Quantize to 256 colors for extreme size reduction while keeping quality decent
            img = img.convert("P", palette=Image.Palette.ADAPTIVE, colors=256)
            img.save(output_path, "PNG", optimize=True)
            print(f"Optimized size: {os.path.getsize(output_path)} bytes")
            return True
    except Exception as e:
        print(f"Error: {e}")
        return False

optimize_hard('src/fs/wallpaper.png', 'src/fs/wallpaper_ultra.png')
