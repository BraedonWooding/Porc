#include "memory.h"

#include <stddef.h>

#define IMPLEMENT_GROWTH_FUNCTION(name, uppercase) \
  inline size_t get_next_capacity_##name(size_t capacity) { \
    return capacity < GROWTH_MIN_##uppercase \
      ? GROWTH_MIN_##uppercase : capacity * GROWTH_RATE_##uppercase; \
  }

IMPLEMENT_GROWTH_FUNCTION(chunk, CHUNK)

