#ifndef PORC_MEMORY_H
#define PORC_MEMORY_H

#define GROWTH_RATE_CHUNK (2)
#define GROWTH_MIN_CHUNK (8)

#define P_ALLOC(bytes) malloc(bytes)
#define P_REALLOC(old, bytes) realloc(old, bytes)
#define P_FREE(ptr) free(ptr)

inline size_t get_next_capacity_chunk(size_t current);

#endif