import sys
import re

hex_file = r"d:\1os copy\animations_WORKING\1os\walkthrough.hex"
out_file = r"d:\1os copy\animations_WORKING\1os\src\fs\video_data.h"

print(f"Reading {hex_file}...")
with open(hex_file, 'r') as f:
    lines = f.readlines()

print(f"Processing lines...")
data = []
# Pattern for hex bytes: exactly 2 hex digits
hex_pat = re.compile(r'^[0-9a-fA-F]{2}$')

for line in lines:
    line = line.strip()
    if not line: continue
    
    # Split by whitespace
    parts = line.split()
    if len(parts) < 2: continue
    
    # Skip the offset (first part)
    # The hex bytes are the ones between the offset and the character representation (if any)
    # Character representation starts after a larger gap usually, but parts.split() collapses it.
    # However, hex bytes are always 2 characters.
    for p in parts[1:]:
        if hex_pat.match(p):
            data.append(f"0x{p}")
        else:
            # Once we hit something that isn't a 2-char hex byte, we stop for this line
            # (handles the ASCII representation at the end)
            break

print(f"Writing {out_file}...")
with open(out_file, 'w') as f:
    f.write("#ifndef VIDEO_DATA_H\n")
    f.write("#define VIDEO_DATA_H\n\n")
    f.write("const unsigned char walkthrough_mpg_data[] = {\n")
    # Write in chunks for readability
    for i in range(0, len(data), 16):
        f.write("    " + ", ".join(data[i:i+16]) + ",\n")
    f.write("};\n\n")
    f.write(f"const unsigned int walkthrough_mpg_size = {len(data)};\n\n")
    f.write("#endif\n")

print(f"Total bytes: {len(data)}")
print("Done.")
