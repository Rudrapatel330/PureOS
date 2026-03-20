import os

# Updated to match actual font-table.h names
real_fonts = {
    'NotoSerifGrantha_Regular_otf': 'mupdf/resources/fonts/noto/NotoSerifGrantha-Regular.otf',
    'SourceHanSerif_Regular_ttc': 'mupdf/resources/fonts/han/SourceHanSerif-Regular.ttc',
    'DroidSansFallbackFull_ttf': 'mupdf/resources/fonts/droid/DroidSansFallbackFull.ttf',
    'NotoEmoji_Regular_ttf': 'mupdf/resources/fonts/noto/NotoEmoji-Regular.ttf'
}

# Add fallback names just in case MuPDF uses slightly different ones
fallbacks = {
    'NotoSerifGranth_Regular_ttc': 'mupdf/resources/fonts/noto/NotoSerifGrantha-Regular.otf'
}

with open('font_list.txt', 'r') as f:
    # Filter out empty lines and comments
    fonts_raw = [line.strip() for line in f if line.strip()]

# Filter out common false positives and empty names
illegal_words = {'symbol', 'name', 'script', 'FORGE', 'NAME', 'NAME2', 'SCRIPT', 'LANG', 'SUBFONT', 'ATTR'}
fonts = []
for f in fonts_raw:
    f_clean = f.strip()
    if f_clean and f_clean not in illegal_words and len(f_clean) > 2:
        fonts.append(f_clean)
fonts = sorted(list(set(fonts)))

# Add fallbacks to fonts list if not already there
for f_name in fallbacks:
    if f_name not in fonts:
        fonts.append(f_name)

with open('src/lib/fonts.asm', 'w') as f:
    f.write('section .rodata\n\n')
    
    for font in fonts:
        if not font or not font.strip():
            print(f"WARNING: Skipping empty font name in list!")
            continue

        symbol_base = f'_binary_{font}'
        
        f.write(f'global {symbol_base}\n')
        f.write(f'global {symbol_base}_start\n')
        f.write(f'global {symbol_base}_end\n')
        f.write(f'global {symbol_base}_size\n')
        
        include_path = real_fonts.get(font) or fallbacks.get(font)
        
        if include_path and os.path.exists(include_path.replace('/', '\\')):
            f.write(f'{symbol_base}:\n')
            f.write(f'{symbol_base}_start:\n')
            f.write(f'    incbin "{include_path}"\n')
            f.write(f'{symbol_base}_end:\n')
            f.write(f'{symbol_base}_size: dq {symbol_base}_end - {symbol_base}_start\n')
        else:
            f.write(f'{symbol_base}:\n')
            f.write(f'{symbol_base}_start:\n')
            f.write(f'    db 0\n')
            f.write(f'{symbol_base}_end:\n')
            f.write(f'{symbol_base}_size: dq 0\n')
        f.write('\n')

print(f"Generated src/lib/fonts.asm with {len(fonts)} fonts.")
for i, f_name in enumerate(fonts[:10]):
    print(f"  {i}: '{f_name}'")
