#ifndef PORC_MEMORY_H
#define PORC_MEMORY_H

#define GROWTH_RATE_CHUNK (2)
#define GROWTH_MIN_CHUNK (8)

#define P_ALLOC(bytes) do { malloc(bytes); } while (0)
#define P_REALLOC(old, bytes) do { realloc(old, bytes); } while (0)
#define P_FREE(ptr) do { free(ptr); } while (0)

inline int get_next_capacity_chunk(int current);

#endif