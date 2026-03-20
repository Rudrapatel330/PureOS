#include "autocomplete.h"
#include "../fs/fs.h"

static const char* commands[] = {
    "help", "info", "clear", "ls", "cat", "del",
    "ren", "copy", "write", "mkdir", "cd", "pwd",
    "reboot", "user", "pagefault", "calc", "rect", 0
};

static int prefix_match(const char* str, const char* prefix, int prefix_len) {
    for (int i = 0; i < prefix_len; i++) {
        char a = str[i], b = prefix[i];
        if (a >= 'a' && a <= 'z') a -= 32;
        if (b >= 'a' && b <= 'z') b -= 32;
        if (a != b) return 0;
        if (a == 0) return 1;
    }
    return 1;
}

int autocomplete(char* input_buf, int* input_pos, int tab_count) {
    int space_pos = -1;
    for (int i = 0; i < *input_pos; i++) {
        if (input_buf[i] == ' ') {
            space_pos = i;
        }
    }
    
    if (space_pos == -1) {
        char* prefix = input_buf;
        int prefix_len = *input_pos;
        if (prefix_len == 0) return 0;
        
        int total_matches = 0;
        for (int i = 0; commands[i]; i++) {
            if (prefix_match(commands[i], prefix, prefix_len)) {
                total_matches++;
            }
        }
        if (total_matches == 0) return 0;
        
        int match_idx = 0;
        for (int i = 0; commands[i]; i++) {
            if (prefix_match(commands[i], prefix, prefix_len)) {
                if (match_idx == (tab_count % total_matches)) {
                    int j = 0;
                    while (commands[i][j]) {
                        input_buf[j] = commands[i][j];
                        j++;
                    }
                    input_buf[j] = 0;
                    *input_pos = j;
                    return 1;
                }
                match_idx++;
            }
        }
    } else {
        char* prefix = input_buf + space_pos + 1;
        int prefix_len = *input_pos - space_pos - 1;
        
        FileInfo files[16];
        int count = fs_list_files("/", files, 16);
        
        int total_matches = 0;
        for (int i = 0; i < count; i++) {
            if (prefix_len == 0 || prefix_match(files[i].name, prefix, prefix_len)) {
                total_matches++;
            }
        }
        if (total_matches == 0) return 0;
        
        int match_idx = 0;
        int target = tab_count % total_matches;
        for (int i = 0; i < count; i++) {
            if (prefix_len == 0 || prefix_match(files[i].name, prefix, prefix_len)) {
                if (match_idx == target) {
                    int pos = space_pos + 1;
                    int j = 0;
                    while (files[i].name[j]) {
                        input_buf[pos++] = files[i].name[j++];
                    }
                    input_buf[pos] = 0;
                    *input_pos = pos;
                    return 1;
                }
                match_idx++;
            }
        }
    }
    return 0;
}
