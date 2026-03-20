#define PL_MPEG_IMPLEMENTATION
#include "video_dev.h"
#include "../fs/vfs.h"
#include "../kernel/heap.h"
#include "../lib/pl_mpeg.h"

extern void print_serial(const char *str);

int video_init() { return 1; }

video_ctx_t *video_open(const char *path) {
  print_serial("VIDEO: Opening ");
  print_serial(path);
  print_serial("\n");

  int fd = vfs_open(path, 0);
  if (fd < 0) {
    print_serial("VIDEO: Failed to open file\n");
    return NULL;
  }

  // Get length using lseek-end trick since we don't have vfs_get_size(fd)
  uint32_t size = vfs_lseek(fd, 0, 2); // SEEK_END
  vfs_lseek(fd, 0, 0);                 // SEEK_SET

  if (size == 0 || size > 64 * 1024 * 1024) { // 64MB Limit
    if (size > 0)
      print_serial("VIDEO: File too large (>64MB)\n");
    vfs_close(fd);
    return NULL;
  }

  void *buf = kmalloc(size);
  if (!buf) {
    print_serial("VIDEO: Failed to allocate file buffer (OOM)\n");
    vfs_close(fd);
    return NULL;
  }

  vfs_read(fd, buf, size);
  vfs_close(fd);

  video_ctx_t *ctx = kmalloc(sizeof(video_ctx_t));
  if (!ctx) {
    kfree(buf);
    return NULL;
  }

  ctx->buffer = buf;
  ctx->size = size;
  ctx->is_playing = 0;

  plm_t *plm = plm_create_with_memory(buf, size, 0);
  if (!plm) {
    print_serial("VIDEO: Failed to create plm instance\n");
    kfree(buf);
    kfree(ctx);
    return NULL;
  }

  ctx->plm = plm;
  ctx->width = plm_get_width(plm);
  ctx->height = plm_get_height(plm);
  ctx->framerate = plm_get_framerate(plm);

  ctx->frame_buffer = kmalloc(ctx->width * ctx->height * 4);
  if (!ctx->frame_buffer) {
    plm_destroy(plm);
    kfree(buf);
    kfree(ctx);
    return NULL;
  }

  ctx->is_playing = 1;
  return ctx;
}

void video_close(video_ctx_t *ctx) {
  if (!ctx)
    return;
  if (ctx->plm)
    plm_destroy((plm_t *)ctx->plm);
  if (ctx->buffer)
    kfree(ctx->buffer);
  if (ctx->frame_buffer)
    kfree(ctx->frame_buffer);
  kfree(ctx);
}

int video_decode_frame(video_ctx_t *ctx) {
  if (!ctx || !ctx->plm)
    return -1;

  plm_frame_t *frame = plm_decode_video((plm_t *)ctx->plm);
  if (!frame) {
    return 0; // EOF or skip
  }

  plm_frame_to_rgba(frame, (uint8_t *)ctx->frame_buffer, ctx->width * 4);
  return 1;
}
