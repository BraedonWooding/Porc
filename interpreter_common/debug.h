#ifndef PORC_DEBUG_H
#define PORC_DEBUG_H

#include "chunk.h"

#include <stdio.h>

void disassembleChunk(Chunk *chunk, FILE *out);
size_t disassembleInstruction(Chunk *chunk, size_t offset, FILE *out);

#endif