#ifndef PORC_HASHTABLE_H
#define PORC_HASHTABLE_H

/*
    This takes quite a bit of influence from the following;
    - https://github.com/anholt/hash_table
    - https://github.com/goldsborough/hashtable
    I needed specific changes to a typical hashtable to
    make it suitable for Porcs purposes
    (the largest is using optionals and results)
*/

/*
    Soln's
    1) Static struct
      - Future calls invalid old data (bug prone) also not amazing
      - Could cache the value easily i.e. cached = *val;
    2) Maybe we need some kind of size_n option???
*/

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#include "option.h"
#include "err.h"
#include "err_type.h"

#define STORE_HASH

typedef bool(*fn_key_eql)(const void *a, const void *b, size_t key_size);
typedef uint32_t(*fn_hash)(const void *key, size_t key_size);

typedef enum {
    ENTRY_FREE,         // this entry has no data associated with it
    ENTRY_DELETED,      // this entry used to have data but it has since been deleted
    ENTRY_OCCUPIED,     // this entry has data
} EntryStatus;

/*
    This is the entry node you get access to.
    The actual nodes are stored in a different format.
*/
typedef struct _map_entry_t {
    EntryStatus status;

#ifdef STORE_HASH
    // for quicker comparison
    uint32_t hash;
#endif
    // private (ish) the ptr into the original node
    void *_node;

    // for key/val
    void *key;
    void *val;
} mapEntry;

typedef struct _map_t {
    // each key size
    const size_t key_size;

    // each value size
    const size_t val_size;

    fn_key_eql eql_cmp;
    fn_hash hash;

    // into prime table
    size_t size_index;

    // size of array of nodes
    size_t size;

    // used to double hash
    size_t rehash;

    // num of max entries
    size_t max_entries;

    size_t entries;
    size_t deleted_entries;

    // to illustrate it is an array
    void *nodes;
} *Map;

Map map_new(size_t key_size, size_t val_size);
void map_free(Map map);

void map_insert(Map map, const void *key, void *value);
void map_insert_hash(Map map, uint32_t hash, const void *key, void *value);
bool map_contains_key(Map map, void *key);
bool map_contains_value(Map map, void *value, fn_key_eql value_comparer);
mapEntry map_lookup(Map map, void *key);
mapEntry map_lookup_hash(Map map, uint32_t hash, void *key);

void map_remove_key(Map map, void *key);
void map_clear(Map map);

bool map_is_empty(Map map);
size_t map_length(Map map);

void map_reserve(Map map, size_t capacity);

mapEntry map_next_entry(Map map, mapEntry prev_entry);

bool map_entry_valid(mapEntry entry);

#define map_null_entry ((mapEntry){0})

#define map_foreach(map, entry) \
    for (mapEntry entry = map_next_entry(map, map_null_entry); \
        map_entry_valid(entry); \
        entry = map_next_entry(map, entry))

#define map_val(entry, type) (*(type*)entry->val)
#define map_key(entry, type) (*(type*)entry->key)

#endif