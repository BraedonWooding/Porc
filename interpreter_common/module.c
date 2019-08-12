#include "module.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void init_module(Mod *mod) {
  mod->loaded_chunks = NULL;
  mod->loaded_chunk_length = 0;
}

Chunk *add_chunk(Mod *mod, Chunk chunk) {
  if (mod->loaded_chunk_length + 1 >= mod->loaded_chunk_capacity) {
    size_t new_size = get_next_capacity_mod(mod->loaded_chunk_capacity + 1);
    mod->loaded_chunk_capacity = new_size;
    void *tmp = P_REALLOC(mod->loaded_chunks, new_size);
    if (tmp == NULL) {
      fprintf(stderr, "Failed to add chunk\n");
      return NULL;
    }
    mod->loaded_chunks = tmp;
  }

  Chunk *to_place = &mod->loaded_chunks[mod->loaded_chunk_length];
  *to_place = chunk;
  mod->loaded_chunk_length++;
  return to_place;
}
