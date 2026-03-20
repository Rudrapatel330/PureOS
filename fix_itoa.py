import os
import re

def fix_file(path):
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Use word boundary to avoid replacing k_itoa
    new_content = re.sub(r'\bitoa\(', 'k_itoa(', content)
    new_content = re.sub(r'\bitoa_hex\(', 'k_itoa_hex(', new_content)
    
    if content != new_content:
        with open(path, 'w', encoding='utf-8', newline='') as f:
            f.write(new_content)
        return True
    return False

src_dir = 'src'
fixed_count = 0
for root, dirs, files in os.walk(src_dir):
    for file in files:
        if file.endswith(('.c', '.h')):
            if fix_file(os.path.join(root, file)):
                fixed_count += 1

print(f"Fixed {fixed_count} files.")
