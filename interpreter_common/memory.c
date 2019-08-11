#include "memory.h"

#define IMPLEMENT_GROWTH_FUNCTION(name, uppercase) \
  inline int get_next_capacity_##name(int capacity) { \
    return capacity < GROWTH_MIN_##uppercase \
      ? GROWTH_MIN_##uppercase : capacity * GROWTH_RATE_##uppercase; \
  }

IMPLEMENT_GROWTH_FUNCTION(chunk, CHUNK)

