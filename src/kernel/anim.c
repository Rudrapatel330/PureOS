#include "anim.h"
// #include <math.h> // No standard lib in kernel

// Simple implementation of math functions if libm not linked
// In kernel environment we usually have to implement these or link libm
static float k_pow(float base, float exp) {
  // Simple integer power for now or use Taylor series if needed
  // For our easing, we mostly need squares/cubes
  if (exp == 2.0f)
    return base * base;

  // For generic pow, return base
  return base;
}

static float k_sin(float x) {
  // Taylor series approximation for sin(x)
  // sin(x) = x - x^3/3! + x^5/5! - x^7/7!
  float t = x;
  float res = t;
  t *= -x * x;
  res += t / 6.0f;
  t *= -x * x;
  res += t / 120.0f;
  t *= -x * x;
  res += t / 5040.0f;
  return res;
}

#define PI 3.1415926535f

float apply_easing(float t, int type) {
  if (t < 0.0f)
    return 0.0f;
  if (t > 1.0f)
    return 1.0f;

  switch (type) {
  case EASE_LINEAR:
    return t;

  case EASE_IN_QUAD:
    return t * t;

  case EASE_OUT_QUAD:
    return t * (2.0f - t);

  case EASE_IN_OUT_QUAD:
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;

  case EASE_OUT_CUBIC: {
    float f = (t - 1.0f);
    return f * f * f + 1.0f;
  }

  case EASE_IN_OUT_CUBIC: {
    if (t < 0.5f) {
      return 4.0f * t * t * t;
    } else {
      float f = -2.0f * t + 2.0f;
      return 1.0f - (f * f * f) / 2.0f;
    }
  }

  case EASE_OUT_EXPO:
    return (t == 1.0f) ? 1.0f : 1.0f - k_pow(2.0f, -10.0f * t);

  case EASE_OUT_ELASTIC: {
    // Spring effect
    // c4 = (2 * PI) / 3
    const float c4 = (2.0f * PI) / 3.0f;
    if (t == 0.0f)
      return 0.0f;
    if (t == 1.0f)
      return 1.0f;
    return k_pow(2.0f, -10.0f * t) * k_sin((t * 10.0f - 0.75f) * c4) + 1.0f;
  }

  case EASE_OUT_BOUNCE: {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1.0f / d1) {
      return n1 * t * t;
    } else if (t < 2.0f / d1) {
      t -= 1.5f / d1;
      return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
      t -= 2.25f / d1;
      return n1 * t * t + 0.9375f;
    } else {
      t -= 2.625f / d1;
      return n1 * t * t + 0.984375f;
    }
  }

  case EASE_OUT_BACK: {
    // Overshoot constant (1.70158 is standard)
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float f = t - 1.0f;
    return 1.0f + c3 * f * f * f + c1 * f * f;
  }
  }
  return t;
}

void anim_tick(animation_t *anim, float dt) {
  if (!anim->active)
    return;

  if (anim->mode == ANIM_MODE_SPRING) {
    // Sub-ticking: break large dt into small 8ms steps for physical stability
    // This prevents "exploding" springs when framerate dips occur.
    float remaining_dt = dt;
    const float step_dt = 0.008f; // 8ms fixed step

    while (remaining_dt > 0.0f) {
      float current_step = (remaining_dt > step_dt) ? step_dt : remaining_dt;

      // Force = -k * x - d * v
      float displacement = anim->current_val - anim->end_val;
      float force =
          -anim->stiffness * displacement - anim->damping * anim->velocity;

      // a = F / m
      float acceleration = force / anim->mass;

      // Integrate
      anim->velocity += acceleration * current_step;
      anim->current_val += anim->velocity * current_step;

      remaining_dt -= step_dt;
    }

    // Stop Condition
    float displacement = anim->current_val - anim->end_val;
    float abs_disp = (displacement < 0) ? -displacement : displacement;
    float abs_vel = (anim->velocity < 0) ? -anim->velocity : anim->velocity;

    if (abs_disp < 0.5f && abs_vel < 1.0f) {
      anim->current_val = anim->end_val;
      anim->active = 0;
      anim->velocity = 0;
      if (anim->on_complete) {
        anim->on_complete(anim->user_data);
      }
    }
  } else {
    // Easing Mode
    anim->elapsed += dt;
    if (anim->elapsed >= anim->duration) {
      anim->elapsed = anim->duration;
      anim->current_val = anim->end_val;
      anim->active = 0;
      if (anim->on_complete) {
        anim->on_complete(anim->user_data);
      }
    } else {
      float progress = anim->elapsed / anim->duration;
      float eased_p = apply_easing(progress, anim->easing_type);
      anim->current_val =
          anim->start_val + (anim->end_val - anim->start_val) * eased_p;
    }
  }

  // Final Safety Check: Prevent NaN or extreme values from breaking the kernel
  if (anim->current_val != anim->current_val || anim->current_val > 10000.0f ||
      anim->current_val < -10000.0f) {
    anim->current_val = anim->end_val;
    anim->velocity = 0;
    anim->active = 0;
  }
}

void anim_init(animation_t *anim) {
  if (!anim)
    return;
  anim->active = 0;
  anim->current_val = 0.0f;
  anim->start_val = 0.0f;
  anim->end_val = 0.0f;
  anim->velocity = 0.0f;
  anim->elapsed = 0.0f;
  anim->on_complete = 0;
  anim->user_data = 0;
}

void anim_init_val(animation_t *anim, float val) {
  if (!anim)
    return;
  anim_init(anim);
  anim->current_val = val;
  anim->start_val = val;
  anim->end_val = val;
}

void anim_start(animation_t *anim, float start, float end, float duration,
                int type) {
  if (!anim)
    return;

  anim->mode = ANIM_MODE_EASE;
  anim->start_val = start;
  anim->end_val = end;
  anim->current_val = start;
  anim->duration = duration;
  anim->elapsed = 0.0f;
  anim->easing_type = type;
  anim->active = 1;

  anim->on_complete = 0;
  anim->user_data = 0;
}

void anim_start_spring(animation_t *anim, float start, float end, float stiff,
                       float damp) {
  if (!anim)
    return;

  anim->mode = ANIM_MODE_SPRING;
  anim->start_val = start;
  anim->end_val = end;
  anim->current_val = start;
  anim->stiffness = stiff;
  anim->damping = damp;
  anim->mass = 1.0f;
  anim->velocity = 0.0f;
  anim->active = 1;

  anim->on_complete = 0;
  anim->user_data = 0;
}

void anim_retarget(animation_t *anim, float new_end) {
  if (!anim)
    return;
  anim->end_val = new_end;
  if (!anim->active) {
    anim->active = 1;
    if (anim->mode == ANIM_MODE_EASE) {
      // If inactive and using ease mode, we kind of need to reset elapsed or
      // start_val But usually retargeting is best for springs
      anim->start_val = anim->current_val;
      anim->elapsed = 0.0f;
    }
  } else {
    if (anim->mode == ANIM_MODE_EASE) {
      // For easing, retargeting is tricky without jumping. We treat current as
      // new start
      float remaining = anim->duration - anim->elapsed;
      if (remaining < 0.05f)
        remaining = 0.05f; // ensure some time
      anim->start_val = anim->current_val;
      anim->elapsed = 0.0f;
      anim->duration = remaining;
    }
  }
}
