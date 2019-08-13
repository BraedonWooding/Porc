#include "debug.h"

static size_t noArgInstruction(const char *name, size_t offset, FILE *out) {
  fprintf(out, "%s\n", name);
  return offset + 1;
}

void disassembleChunk(Chunk *chunk, FILE *out) {
  fprintf(out, "== Chunk (%zu/%zu) ==", chunk->count, chunk->capacity);
  size_t offset = 0;
  while (offset < chunk->count) {
      offset = disassembleInstruction(chunk, offset, out);
  }
}

size_t disassembleInstruction(Chunk *chunk, size_t offset, FILE *out) {
  fprintf(out, "%04zu ", offset);
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4zu ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    default:
      fprintf(out, "??? %d", instruction);
      return offset + 1;
  }
}

