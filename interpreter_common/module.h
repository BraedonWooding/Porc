#ifndef PORC_MODULE_H
#define PORC_MODULE_H

#ifdef __cplusplus
extern "C" {
#else
typedef struct Mod Mod;
#endif

#include "porc_common.h"
#include "chunk.h"

/*
	Represents a compiled module.
*/
struct Mod {
  size_t loaded_chunk_capacity;
  size_t loaded_chunk_length;
  struct Chunk *loaded_chunks;


};

void init_module(Mod *mod);

struct Chunk *add_chunk(Mod *mod, struct Chunk chunk);

#ifdef __cplusplus
}
#endif

#endif