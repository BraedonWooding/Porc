#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "map.h"
#include "obsidian.h"
#include "option.h"

#define DEFAULT_SIZE (8)

/*
    @NOTE: this is very very wrong
    Honestly the idea is great but I need to plan it out better
*/

uint32_t default_hash(const void *key, size_t key_size);
uint32_t string_hash(const void *key, size_t key_size);
bool default_cmp(const void *a, const void *b, size_t key_size);
bool string_cmp(const void *a, const void *b, size_t key_size);

typedef struct _map_node_t {
    EntryStatus status;
#ifdef STORE_HASH
    // for quicker comparison
    uint32_t hash;
#endif
} *MapNode;

#define DICT_ENTRY_SIZE(map) (sizeof(struct _map_node_t) + (map)->key_size + (map)->val_size)
#define DICT_NODE_KEY(map, node) ((void*)(&(node)->hash + 1))
#define DICT_NODE_VAL(map, node) ((void*)((size_t)(&(node)->hash + 1) + (map)->key_size))

/*
    Private funcs
*/

static void rehash(Map map, int new_size_index);
static void rehash_same_size(Map map);

#define INDEX_ENTRIES(map, i) ((MapNode)((size_t)map->nodes + i * DICT_ENTRY_SIZE(map)))

#define HASH_SIZES_LEN (31)

/*
    Just some primes for rehashing, taken from:
    https://github.com/anholt/hash_table/blob/master/hash_table.c
    Original source is Knuth.
*/
static const struct {
    size_t max_entries, size, rehash;
} hash_sizes[HASH_SIZES_LEN] = {
    { 2,            5,            3         },
    { 4,            7,            5         },
    { 8,            13,           11        },
    { 16,           19,           17        },
    { 32,           43,           41        },
    { 64,           73,           71        },
    { 128,          151,          149       },
    { 256,          283,          281       },
    { 512,          571,          569       },
    { 1024,         1153,         1151      },
    { 2048,         2269,         2267      },
    { 4096,         4519,         4517      },
    { 8192,         9013,         9011      },
    { 16384,        18043,        18041     },
    { 32768,        36109,        36107     },
    { 65536,        72091,        72089     },
    { 131072,       144409,       144407    },
    { 262144,       288361,       288359    },
    { 524288,       576883,       576881    },
    { 1048576,      1153459,      1153457   },
    { 2097152,      2307163,      2307161   },
    { 4194304,      4613893,      4613891   },
    { 8388608,      9227641,      9227639   },
    { 16777216,     18455029,     18455027  },
    { 33554432,     36911011,     36911009  },
    { 67108864,     73819861,     73819859  },
    { 134217728,    147639589,    147639587 },
    { 268435456,    295279081,    295279079 },
    { 536870912,    590559793,    590559791 },
    { 1073741824,   1181116273,   1181116271},
    { 2147483648ul, 2362232233ul, 2362232231ul}
};

static mapEntry new_entry_from_node(Map map, MapNode node) {
    return (mapEntry) {
        .status = node->status,
#ifdef STORE_HASH
        .hash = node->hash,
#endif
        .key = DICT_NODE_KEY(map, node),
        .val = DICT_NODE_VAL(map, node),
        ._node = node,
    };
}

Map map_new(size_t key_size, size_t val_size) {
    Map map = malloc(sizeof(struct _map_t));
    if (map == NULL) return NULL;
    *(int*)map->key_size = key_size;
    *(int*)map->val_size = val_size;
    map->hash = default_hash;
    map->eql_cmp = default_cmp;
    map->size_index = 0;
    map->rehash = hash_sizes[map->size_index].rehash;
    map->max_entries = hash_sizes[map->size_index].max_entries;
    map->size = hash_sizes[map->size_index].size;
    map->nodes = calloc(map->size, DICT_ENTRY_SIZE(map));
    if (map->nodes == NULL) {
        free(map);
        return NULL;
    }

    return map;
}

void map_free(Map map) {
    free(map->nodes);
    free(map);
}

void map_increase_capacity(Map map) {

}

void map_insert(Map map, const void *key, void *value) {
    return map_insert_hash(map, map->hash(key, map->key_size), key, value);
}

void map_insert_hash(Map map, uint32_t hash, const void *key, void *value) {
    // check if we need more space
    if (map->entries >= map->max_entries) rehash(map, map->size_index + 1);
    if (map->deleted_entries + map->entries >= map->max_entries) rehash_same_size(map);

    uint32_t start_hash_index = hash % map->size;
    uint32_t hash_index = start_hash_index;

    MapNode available = NULL;
    do {
        MapNode entry = INDEX_ENTRIES(map, hash_index);
        if (entry->status != ENTRY_OCCUPIED) {
            // @BENCH: Could it be more efficient to find the last ??
            // Pro: It would mean that future collisions of this 'type' are cheaper
            // Con: It would mean that future noncollisions could be more expensive
            // I'm leaning towards the con being stronger so finding first is better
            // Again this may make no meaningful impact but mapionaries can get very big
            // and we will use this impl for porc std lib too most likely so it would
            // be nice to have 
            if (available == NULL) available = entry;
            // @TEST: I'm not sure we can have this here
            // this is because you could have the case where a rehash_same_size
            // occurs which would flag all the deleted entries as free again
            // which would than cause this to break prematurely and add two of the same entries
            if (entry->status == ENTRY_FREE) break;
        }

        if (entry->status == ENTRY_OCCUPIED && entry->hash == hash &&
             map->eql_cmp(key, DICT_NODE_KEY(map, entry), map->key_size)) {
            memcpy(DICT_NODE_KEY(map, entry), key, map->key_size);
            memcpy(DICT_NODE_VAL(map, entry), value, map->val_size);
            return;
        }

        hash_index = ((1 + hash_index % map->rehash) + hash_index) % map->size;
    } while (hash_index != start_hash_index);

    if (available != NULL) {
        if (available->status == ENTRY_DELETED) map->deleted_entries--;
        available->hash = hash;
        memcpy(DICT_NODE_KEY(map, available), key, map->key_size);
        memcpy(DICT_NODE_VAL(map, available), value, map->val_size);
        map->entries++;
        return;
    }

    // Shouldn't reach here unless resize failed which we should detect earlier
    assert(NULL);
}

bool map_contains_key(Map map, void *key) {
    return map_lookup(map, key).status == ENTRY_OCCUPIED;
}

bool map_contains_value(Map map, void *value, fn_key_eql value_comparer) {
    map_foreach(map, entry) {
        if (value_comparer(entry.val, value, map->val_size)) return true;
    }
    return false;
}

mapEntry map_lookup(Map map, void *key) {
    uint32_t hash = map->hash(key, map->key_size);
    return map_lookup_hash(map, hash, key);
}

mapEntry map_lookup_hash(Map map, uint32_t hash, void *key) {
    uint32_t start_hash_index = hash % map->size;
    uint32_t hash_index = start_hash_index;

    do {
        MapNode entry = INDEX_ENTRIES(map, hash_index);
        if (entry->status == ENTRY_FREE) return map_null_entry;
        // this is taking the presumption that hash checking is faster
        // by enough mileage to include it before the key cmp function
        if (entry->status == ENTRY_OCCUPIED && entry->hash == hash &&
             map->eql_cmp(key, DICT_NODE_KEY(map, entry), map->key_size)) {
            return new_entry_from_node(map, entry);
        }

        // double hash the entry till we either get to a free point
        // (and thus there is no more to go through) or we run out of
        // possible spaces and thus can stop.
        hash_index = ((1 + hash_index % map->rehash) + hash_index) % map->size;
    } while (hash_index != start_hash_index);
    return map_null_entry;
}

// this is a cheaper rehash in the case where we want to just rehash our current size
static void rehash_same_size(Map map) {
    struct _map_entry_t *new_nodes = calloc(map->size, DICT_ENTRY_SIZE(map));
    // @Q: again, we could use Result to support OOM better
    if (new_nodes == NULL) return;

    // do a copy of old state then update new state
    struct _map_t old_map = *map;
    map->nodes = new_nodes;
    map->entries = 0;
    map->deleted_entries = 0;

    // move over nodes then cleanup old
    map_foreach(&old_map, entry) {
        map_insert_hash(map, entry.hash, entry.key, entry.val);
    }
    free(old_map.nodes);
}

static void rehash(Map map, int new_size_index) {
    // out of space
    // @Q: should we abort??  Or have some sort of err?
    // since this will just cause undefined behaviour??
    if (new_size_index >= HASH_SIZES_LEN) return;

    size_t new_size = hash_sizes[new_size_index].size;
    struct _map_entry_t *new_nodes = calloc(new_size, DICT_ENTRY_SIZE(map));
    // @Q: again, we could use Result to support OOM better
    if (new_nodes == NULL) return;

    // do a copy of old state then update new state
    struct _map_t old_map = *map;
    map->nodes = new_nodes;
    map->size_index = new_size_index;
    map->size = new_size;
    map->rehash = hash_sizes[new_size_index].rehash;
    map->max_entries = hash_sizes[new_size_index].max_entries;
    map->entries = 0;
    map->deleted_entries = 0;

    // move over nodes then cleanup old
    map_foreach(&old_map, entry) {
        map_insert_hash(map, entry.hash, entry.key, entry.val);
    }
    free(old_map.nodes);
}

void map_remove_key(Map map, void *key);
void map_clear(Map map);

bool map_is_empty(Map map);
size_t map_length(Map map);

void map_reserve(Map map, size_t capacity);

bool map_entry_valid(mapEntry entry) {
    return entry.status == ENTRY_OCCUPIED;
}

mapEntry map_next_entry(Map map, mapEntry prev_entry) {
    size_t entry_addr = map_entry_valid(prev_entry) ? (size_t)map->nodes : (size_t)prev_entry._node + DICT_ENTRY_SIZE(map);

    // technically invalid, but since we aren't dereferencing it is fine
    MapNode last_entry = INDEX_ENTRIES(map, map->size);
    size_t last_entry_addr = (size_t)last_entry;

    for (; entry_addr < last_entry_addr; entry_addr += DICT_ENTRY_SIZE(map)) {
        MapNode entry = (MapNode)entry_addr;
        if (entry->status == ENTRY_OCCUPIED) return new_entry_from_node(map, entry);
    }
    return map_null_entry;
}
