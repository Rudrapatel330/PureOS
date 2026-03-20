#ifndef VIDEO_DEV_H
#define VIDEO_DEV_H

#include <stddef.h>
#include <stdint.h>


// Video context structure
typedef struct {
  void *plm;    // plm_t pointer
  void *buffer; // File buffer
  size_t size;  // File size
  uint32_t width;
  uint32_t height;
  float framerate;
  int is_playing;
  uint32_t *frame_buffer; // Decoded ARGB32 frame
} video_ctx_t;

// Initialize video system
int video_init();

// Open a video file
video_ctx_t *video_open(const char *path);

// Close video
void video_close(video_ctx_t *ctx);

// Decode next frame
// Returns 1 if a new frame was decoded, 0 if no new frame, -1 on error/EOF
int video_decode_frame(video_ctx_t *ctx);

#endif
