from PIL import Image
import os

def resize_png(input_path, output_path, width, height):
    try:
        with Image.open(input_path) as img:
            # High quality resize
            img = img.resize((width, height), Image.Resampling.LANCZOS)
            img.save(output_path, "PNG", optimize=True)
            print(f"Successfully resized to {width}x{height}")
            return True
    except Exception as e:
        print(f"Error: {e}")
        return False

resize_png('src/fs/wallpaper.png', 'src/fs/wallpaper_optimized.png', 1024, 768)
