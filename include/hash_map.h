#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdint.h>
#include <stdlib.h>
#include "uthash.h"
#include "block.h"

typedef struct hash_map_entry {
    hash_t key;              // block hash
    BlockNode *value;        // pointer to block node
    UT_hash_handle hh;
} hash_map_t;

void hash_map_put(hash_map_t **map, const hash_t key, BlockNode *value);
BlockNode *hash_map_get(hash_map_t **map, const hash_t key);
void hash_map_remove(hash_map_t **map, const hash_t key);
void hash_map_free(hash_map_t **map);

#endif

