with open(r'd:\1os\src\kernel\desktop.c', 'r') as f:
    lines = f.readlines()

# Find and fix line 7 (index 6) which has literal \n sequences
new_lines = []
for line in lines:
    if 'ctxmenu.h' in line and 'extern' in line:
        # This is the malformed line - split it properly
        new_lines.append('#include "../ui/ctxmenu.h"\n')
        new_lines.append('\n')
        new_lines.append('// Reference wallpaper data from ramfs.c (avoid duplicating 175KB)\n')
        new_lines.append('extern const unsigned char wallpaper_bmp_data[];\n')
        new_lines.append('extern const unsigned int wallpaper_bmp_size;\n')
    else:
        new_lines.append(line)

with open(r'd:\1os\src\kernel\desktop.c', 'w') as f:
    f.writelines(new_lines)

print('Fixed desktop.c!')
