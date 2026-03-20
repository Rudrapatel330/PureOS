#include "../../kernel/syscall.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Simple Package Manager (pkg)
// Usage: pkg list
//        pkg install <name>

void print_help() {
  printf("PureOS Package Manager (pkg) v1.0\n");
  printf("Usage: pkg list | install <name>\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_help();
    return 1;
  }

  if (strcmp(argv[1], "list") == 0) {
    printf("Fetching repository data from /repo...\n");
    int fd = vfs_open("/repo", 0x1);
    if (fd >= 0) {
      for (int i = 0;; i++) {
        vfs_node_t *node = vfs_readdir(fd, i);
        if (!node)
          break;
        if (strstr(node->name, ".pkg")) {
          printf("- %s\n", node->name);
        }
        kfree(node);
      }
      vfs_close(fd);
    } else {
      printf("Error: Repository /repo not found.\n");
    }
  } else if (strcmp(argv[1], "install") == 0) {
    if (argc < 3) {
      printf("Error: No package name specified.\n");
      return 1;
    }
    char path[128];
    sprintf(path, "/repo/%s.pkg", argv[2]);
    printf("Installing package: %s...\n", argv[2]);

    int fd = vfs_open(path, 0x1);
    if (fd < 0) {
      printf("Error: Package %s not found in /repo.\n", argv[2]);
      return 1;
    }

    // In a real OS, we'd parse the .pkg archive.
    // Here we'll just simulate extraction.
    printf("Extracting files to /usr/bin/...\n");
    printf("Package %s installed successfully.\n", argv[2]);
    vfs_close(fd);
  } else {
    print_help();
  }

  return 0;
}
