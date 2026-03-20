#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// PureBox: Minimal Multi-call Binary for PureOS

int cmd_ls(int argc, char **argv) {
  printf("PureBox: ls - Listing directories...\n");
  // Implementation: Use SYS_OPEN and SYS_READ (or a specialized DIRENT syscall)
  return 0;
}

int cmd_cat(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: cat <file>\n");
    return 1;
  }
  printf("PureBox: cat - Reading %s...\n", argv[1]);
  return 0;
}

int cmd_echo(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 1)
    return 1;

  // Get the base name of the executable (from argv[0])
  char *cmd = argv[0];
  char *slash = strrchr(cmd, '/');
  if (slash)
    cmd = slash + 1;

  if (strcmp(cmd, "ls") == 0)
    return cmd_ls(argc, argv);
  if (strcmp(cmd, "cat") == 0)
    return cmd_cat(argc, argv);
  if (strcmp(cmd, "echo") == 0)
    return cmd_echo(argc, argv);
  if (strcmp(cmd, "purebox") == 0) {
    if (argc < 2) {
      printf("PureBox v1.0 - Multi-call binary\n");
      printf("Supported commands: ls, cat, echo\n");
      return 0;
    }
    // Shift arguments and recurse? Or just re-match
    argv++;
    argc--;
    return main(argc, argv);
  }

  printf("PureBox: Unknown command '%s'\n", cmd);
  return 1;
}
