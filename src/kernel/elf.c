#include "elf.h"
#include "../fs/vfs.h"
#include "heap.h"
#include "paging.h"
#include "string.h"
#include "task.h"

extern void print_serial(const char *);

int elf_load_into_process(task_t *t, const char *path) {
  int fd = vfs_open(path, 0x01);
  if (fd < 0) {
    print_serial("ELF: Failed to open file ");
    print_serial(path);
    print_serial("\n");
    return 0;
  }

  elf64_header_t header;
  if (vfs_read(fd, (uint8_t *)&header, sizeof(elf64_header_t)) !=
      sizeof(elf64_header_t)) {
    print_serial("ELF: Failed to read header\n");
    vfs_close(fd);
    return 0;
  }

  // Validate MAGIC
  if (*(uint32_t *)header.e_ident != ELF_MAGIC) {
    print_serial("ELF: Invalid magic\n");
    vfs_close(fd);
    return 0;
  }

  char interp_path[128];
  int has_interp = 0;

  for (int i = 0; i < header.e_phnum; i++) {
    elf64_program_header_t ph;
    vfs_lseek(fd, header.e_phoff + i * header.e_phentsize, 0); // SEEK_SET
    vfs_read(fd, (uint8_t *)&ph, sizeof(elf64_program_header_t));

    if (ph.p_type == PT_INTERP) {
      if (ph.p_filesz < 128) {
        vfs_lseek(fd, ph.p_offset, 0);
        vfs_read(fd, (uint8_t *)interp_path, ph.p_filesz);
        interp_path[ph.p_filesz] = 0;
        has_interp = 1;
      }
    } else if (ph.p_type == PT_LOAD) {
      uint64_t num_pages = (ph.p_memsz + 4095) / 4096;
      uint32_t seg_phys;
      void *seg_buf = kmalloc_ap((uint32_t)(num_pages * 4096), &seg_phys);
      memset(seg_buf, 0, (uint32_t)(num_pages * 4096));

      vfs_lseek(fd, ph.p_offset, 0); // SEEK_SET
      vfs_read(fd, (uint8_t *)seg_buf, (uint32_t)ph.p_filesz);

      for (uint64_t j = 0; j < num_pages * 4096; j += 4096) {
        paging_map_user_page(t->pagedir, ph.p_vaddr + j, seg_phys + j,
                             0x7); // User, RW, Present
      }
    }
  }

  vfs_close(fd);

  if (has_interp) {
    print_serial("ELF: Program has interpreter: ");
    print_serial(interp_path);
    print_serial("\n");
    // Load interpreter (recursion!)
    return elf_load_into_process(t, interp_path);
  }

  t->rip = header.e_entry;
  return 1;
}

int elf_load_file(const char *path) {
  int fd = vfs_open(path, 0x01);
  if (fd < 0)
    return 0;

  elf64_header_t header;
  vfs_read(fd, (uint8_t *)&header, sizeof(elf64_header_t));
  vfs_close(fd);

  task_t *t = create_user_process(path, (void *)(uintptr_t)header.e_entry);
  if (!t)
    return 0;
  return elf_load_into_process(t, path);
}
