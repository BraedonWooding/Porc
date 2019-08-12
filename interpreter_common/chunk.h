#ifndef PORC_CHUNK_H
#define PORC_CHUNK_H

#ifdef __cplusplus
extern "C" {
#else
typedef struct Chunk Chunk;
#endif

#include "porc_common.h"

typedef size_t LineData;

struct Chunk {
  size_t count;
  size_t capacity;
  uint8_t *code;
  LineData *lines;
};

void init_chunk(Chunk *chunk);
void init_chunk_guess(Chunk *chunk, size_t size);
void grow_chunk(Chunk *chunk, size_t new_size);
void write_chunk(Chunk *chunk, byte byte, LineData line);
void write_chunk_seq(Chunk *chunk, size_t len, byte *bytes, LineData line);
void empty_chunk(Chunk *chunk);

#define WRITE_CHUNK_RAW(chunk, data, line) \
  write_chunk_seq(chunk, sizeof(data), (byte*)&data, line)

#ifdef __cplusplus
}
#endif

#endif