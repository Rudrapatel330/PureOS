import re
import os

def refactor_file(filepath, app_name):
    if not os.path.exists(filepath):
        print(f"Skipping {filepath}, not found.")
        return
        
    print(f"Refactoring {filepath} ({app_name})...")
    with open(filepath, 'r') as f:
        data = f.read()

    # Ensure ui_layout.h is included
    if '#include "../kernel/ui_layout.h"' not in data:
        data = "#include \"../kernel/ui_layout.h\"\n" + data

    # 1. Inject layout helper functions if they belong to known sidebar apps
    if 'get_sidebar_width' not in data:
        if app_name == 'settings':
            helpers = r'''
static int get_sidebar_width(void) {
    int fs = ui_get_font_scale();
    return ui_measure_text_width("\xF0\x9F\x8E\xA8 Personalization", fs) + 60;
}
static int get_cx(void) {
    return get_sidebar_width() + 30;
}
'''
            include_match = list(re.finditer(r'#include.*?\n', data))
            if include_match:
                insert_pos = include_match[-1].end()
                data = data[:insert_pos] + helpers + data[insert_pos:]
        elif app_name == 'explorer' or app_name == 'filemgr':
            helpers = r'''
static int get_sidebar_width(void) {
    int fs = ui_get_font_scale();
    return ui_measure_text_width("External Drive", fs) + 60;
}
'''
            include_match = list(re.finditer(r'#include.*?\n', data))
            if include_match:
                insert_pos = include_match[-1].end()
                data = data[:insert_pos] + helpers + data[insert_pos:]

    # 2. Replace hardcoded SIDEBAR_WIDTH / SW / CX / SIDE_WIDTH
    if app_name == 'settings':
        data = re.sub(r'#define SW \d+', '', data)
        data = re.sub(r'#define CX \(SW \+ 30\)', '', data)
        data = re.sub(r'\bSW\b', 'get_sidebar_width()', data)
        data = re.sub(r'\bCX\b', 'get_cx()', data)
    elif app_name in ['explorer', 'filemgr']:
        data = re.sub(r'#define SIDEBAR_WIDTH \d+', '', data)
        data = re.sub(r'\bSIDEBAR_WIDTH\b', 'get_sidebar_width()', data)
    elif app_name == 'chat':
        data = re.sub(r'#define SIDE_WIDTH \d+', 'static int get_sidebar_width(void) { return ui_get_font_scale() * 10; }', data)
        data = re.sub(r'\bSIDE_WIDTH\b', 'get_sidebar_width()', data)
    elif app_name == 'recorder':
        data = re.sub(r'#define SIDEBAR_WIDTH \d+', 'static int get_sidebar_width(void) { return ui_get_font_scale() * 15; }', data)
        data = re.sub(r'\bSIDEBAR_WIDTH\b', 'get_sidebar_width()', data)

    # 3. Dynamic vertical spacing & element sizing
    if app_name == 'settings':
        # Sidebar Tabs
        data = re.sub(r'int ty = 45 \+ i \* 50;', r'int item_h = fs + 24; int ty = 45 + i * (item_h + 10);', data)
        data = re.sub(r'winmgr_draw_rounded_rect_ex\(win, 15, ty, get_sidebar_width\(\) - 30, 40,', r'winmgr_draw_rounded_rect_ex(win, 15, ty, get_sidebar_width() - 30, item_h,', data)
        data = re.sub(r'winmgr_draw_text\(win, 30, ty \+ 12,', r'winmgr_draw_text(win, 30, ty + (item_h - fs) / 2,', data)
        
        # General replacements for card heights
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )45', r'\1(fs * 2 + 10)', data)
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )50', r'\1(fs * 2 + 15)', data)
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )75', r'\1(fs * 3 + 20)', data)
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )80', r'\1(fs * 3 + 25)', data)
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )100', r'\1(fs * 4 + 30)', data)
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )120', r'\1(fs * 5 + 40)', data)
        data = re.sub(r'(DRAW_ROUNDED_CARD\(.*?card_w, )140', r'\1(fs * 6 + 50)', data)
        
        # Inner card text alignment
        data = data.replace('cy + 15', 'cy + (fs * 0.5)')
        data = data.replace('cy + 20', 'cy + 10')
        data = data.replace('cy + 45', 'cy + 10 + fs + 8')
        data = data.replace('cy + 35', 'cy + 10 + fs + 4')
        
    elif app_name in ['explorer', 'filemgr']:
        data = data.replace('y += 20;', 'y += fs + 4;')
        data = data.replace('y += 28;', 'y += fs + 12;')
        data = re.sub(r'(winmgr_fill_rect\(.*?, )24', r'\1(fs + 8)', data)
        data = data.replace('y + 2', 'y + 4')
        data = data.replace(', 50, COL_CONTENT_BG)', ', fs + 34, COL_CONTENT_BG)')
        data = data.replace('int ty = 50;', 'int ty = fs + 34;')
        data = data.replace(', 60, COL_CONTENT_BG)', ', fs + 44, COL_CONTENT_BG)')

    # 4. Global Injection of 'fs' (font scale) variable in draw functions
    # This ensures that any function using 'fs' has it defined.
    functions = re.split(r'(\n(?:static |void ).*?\(.*?\)\s*\{)', data)
    for i in range(1, len(functions), 2):
        body = functions[i+1]
        # Detect if 'fs' is used but not defined in this scope
        if 'fs' in body and 'int fs =' not in body and 'fs_' not in body and '->fs' not in body:
            functions[i+1] = '\n  int fs = ui_get_font_scale();' + body
    data = "".join(functions)

    with open(filepath, 'w') as f:
        f.write(data)

def main():
    apps_dir = 'src/apps'
    exclude = ['browser.c', 'doom', 'mail', 'autocomplete.c', 'ld-pureos.c', 'pkg.c', 'purebox.c']
    
    for filename in os.listdir(apps_dir):
        if filename in exclude or filename.endswith('.h'):
            continue
            
        filepath = os.path.join(apps_dir, filename)
        if os.path.isfile(filepath):
            app_name = filename.replace('.c', '')
            refactor_file(filepath, app_name)

if __name__ == "__main__":
    main()
    print("Bulk Refactoring complete.")
