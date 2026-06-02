from PIL import Image, ImageDraw, ImageFont
import os

def generate_font():
    font_path = 'C:\\Windows\\Fonts\\segoeui.ttf'
    if not os.path.exists(font_path):
        font_path = 'C:\\Windows\\Fonts\\arial.ttf'
        
    font = ImageFont.truetype(font_path, 15)

    out_c = '#include <stdint.h>\n\n'
    out_c += 'const uint8_t font16x16_aa[256][16][16] = {\n'
    widths = []
    
    # Pre-calculate global vertical crop area
    crop_y_start = 4
    
    for c in range(256):
        canvas = Image.new('L', (32, 32), color=0)
        draw = ImageDraw.Draw(canvas)
        
        char = chr(c) if c >= 32 and c < 127 else ' '
        if c >= 32 and c < 127:
            draw.text((0, 0), char, font=font, fill=255)
            pixels = list(canvas.getdata())
            min_x, max_x = 32, 0
            
            # Find horizontal bounds
            for py in range(32):
                for px in range(32):
                    if pixels[py * 32 + px] > 5:
                        if px < min_x: min_x = px
                        if px > max_x: max_x = px
            
            if max_x >= min_x:
                glyph_w = max_x - min_x + 1
                
                # Crop horizontally exactly to bounds, vertically to global fixed range
                # to preserve baseline!
                glyph_img = canvas.crop((min_x, crop_y_start, max_x + 1, crop_y_start + 16))
                
                final = Image.new('L', (16, 16), color=0)
                
                # Left align with 1px padding
                x_off = 1
                if glyph_w > 14:
                    # Squeeze horizontally if it exceeds 14px width
                    glyph_img = glyph_img.resize((14, 16), Image.LANCZOS)
                    glyph_w = 14
                    
                final.paste(glyph_img, (x_off, 0))
                
                # Add 1px right padding to the advance width
                widths.append(x_off + glyph_w + 1)
            else:
                final = Image.new('L', (16, 16), color=0)
                widths.append(5 if c == 32 else 8)
        else:
            final = Image.new('L', (16, 16), color=0)
            widths.append(5 if c == 32 else 8)
            
        pixels = list(final.getdata())
        out_c += f'  // Char {c}\n  {{\n'
        for y in range(16):
            out_c += '    {'
            for x in range(16):
                out_c += f'{pixels[y * 16 + x]:3},'
            out_c += '},\n'
        out_c += '  },\n'
        
    out_c += '};\n\n'
    out_c += 'const uint8_t font16x16_widths[256] = {\n  '
    for i, w in enumerate(widths):
        out_c += f'{w}, '
        if i % 16 == 15: out_c += '\n  '
    out_c += '\n};\n'
    
    with open('src/gui/font16_aa.c', 'w') as f:
        f.write(out_c)

if __name__ == '__main__':
    generate_font()
