#ifndef ANIM_H
#define ANIM_H

#include <stdint.h>

// Easing Types
#define EASE_LINEAR 0
#define EASE_IN_QUAD 1
#define EASE_OUT_QUAD 2
#define EASE_IN_OUT_QUAD 3
#define EASE_OUT_CUBIC 6
#define EASE_IN_OUT_CUBIC 8
#define EASE_OUT_EXPO 7
#define EASE_OUT_ELASTIC 4 // Spring
#define EASE_OUT_BOUNCE 5
#define EASE_OUT_BACK 9   // Slight overshoot then settle

#define ANIM_MODE_EASE 0
#define ANIM_MODE_SPRING 1

// Standard Spring Constants (Mass=1.0)
// Lower stiffness (K) = slower animation (macOS is around 0.4s duration)
// Critical damping (D) = ~2*sqrt(K)
#define SPRING_STIFF_K 300.0f
#define SPRING_STIFF_D 35.0f
#define SPRING_SOFT_K 200.0f
#define SPRING_SOFT_D 30.0f
#define SPRING_BOUNCY_K 250.0f
#define SPRING_BOUNCY_D 10.0f
#define SPRING_SNAPPY_K 350.0f
#define SPRING_SNAPPY_D 28.0f

typedef struct {
  int mode; // ANIM_MODE_EASE or ANIM_MODE_SPRING
  float start_val;
  float end_val;
  float current_val;

  // Ease Mode
  float duration; // In seconds
  float elapsed;  // In seconds
  int easing_type;

  // Spring Mode
  float velocity;
  float stiffness; // k
  float damping;   // d
  float mass;      // usually 1.0

  int active;

  void (*on_complete)(void *user_data);
  void *user_data;
} animation_t;

// Global animation tick (call this every frame with delta time in seconds)
void anim_tick(animation_t *anim, float dt);

// Initialize an animation structure
void anim_init(animation_t *anim);
void anim_init_val(animation_t *anim, float val);

// Create/Start an animation
void anim_start(animation_t *anim, float start, float end, float duration,
                int type);
// Spring:
void anim_start_spring(animation_t *anim, float start, float end, float stiff,
                       float damp);

void anim_retarget(animation_t *anim, float new_end);

// Math helper
float apply_easing(float t, int type);

#endif
