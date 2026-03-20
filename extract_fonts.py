import re

with open('mupdf/source/fitz/font-table.h', 'r') as f:
    content = f.read()

# Match FONT(forge, name, ...) or DEFNOTO(name, ...)
# Groups: 1=Name for FONT, 2=Name for DEFNOTO
matches = re.findall(r'FONT\s*\(\s*\w+\s*,\s*(\w+)|DEFNOTO\s*\(\s*(\w+)', content)

fonts = []
for m in matches:
    name = m[0] or m[1]
    if name:
        fonts.append(name)

# Dedup
fonts = sorted(list(set(fonts)))

with open('font_list.txt', 'w') as f:
    for font in fonts:
        f.write(font + '\n')

print(f"Extracted {len(fonts)} fonts to font_list.txt")
