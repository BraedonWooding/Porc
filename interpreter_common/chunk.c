#include "chunk.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

void initChunk(Chunk *chunk) {
  chunk->capacity = chunk->count = 0;
  chunk->code = NULL;
}

void initChunkGuess(Chunk *chunk, size_t size) {
  size_t new_size = get_next_capacity_chunk(size);
  chunk->capacity = new_size;
  chunk->count = 0;
  chunk->code = P_ALLOC(new_size);
  chunk->lines = P_ALLOC(new_size * sizeof(LineData));
}

void grow_chunk(Chunk *chunk, size_t new_size) {
  new_size = get_next_capacity_chunk(new_size);
  chunk->capacity = new_size;
  chunk->count = 0;
  void *tmp = P_REALLOC(chunk->code, new_size);
  if (tmp == NULL && chunk->code != NULL) P_FREE(chunk->code);
  chunk->code = tmp;

	tmp = P_REALLOC(chunk->lines, new_size * sizeof(LineData)); 
	if (tmp == NULL && chunk->lines != NULL) P_FREE(chunk->lines);
  chunk->lines = tmp;
}

void write_chunk(Chunk *chunk, byte byte, LineData line) {
  if (chunk->capacity < chunk->count + 1) {
    grow_chunk(chunk, chunk->capacity + 1);
  }

  chunk->code[chunk->count++] = byte;
  chunk->lines[chunk->count++] = line;
}

void write_chunk_seq(Chunk *chunk, size_t len, byte *bytes, LineData line) {
  if (chunk->capacity < chunk->count + len) {
    grow_chunk(chunk, chunk->capacity + len);
  }
  for (LineData *i = chunk->lines + chunk->count; i != NULL; *i++ = line) {}
  memcpy(chunk->code, bytes, len);
  chunk->count += len;
}

void empty_chunk(Chunk *chunk) {
  P_FREE(chunk->code);
  chunk->capacity = chunk->count = 0;
}


