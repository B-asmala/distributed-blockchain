#include "hash_map.h"


void hash_map_put(hash_map_t **map, const hash_t key, BlockNode *value) {
    hash_map_t *e;

    HASH_FIND(hh, *map, key, sizeof(hash_t), e);
    if (!e) {
        e = malloc(sizeof(*e));
        memcpy(e->key, key, sizeof(hash_t));   
        HASH_ADD(hh, *map, key, sizeof(hash_t), e);
    }

    e->value = value;
}

BlockNode *hash_map_get(hash_map_t **map, const hash_t key) {
    hash_map_t *e;
    HASH_FIND(hh, *map, key, sizeof(hash_t), e);
    return e ? e->value : NULL;
}

void hash_map_remove(hash_map_t **map, const hash_t key) {
    hash_map_t *e;
    HASH_FIND(hh, *map, key, sizeof(hash_t), e);
    if (e) {
        HASH_DEL(*map, e);
        free(e);
    }
}

void hash_map_free(hash_map_t **map) {
    hash_map_t *cur, *tmp;
    HASH_ITER(hh, *map, cur, tmp) {
        HASH_DEL(*map, cur);
        free(cur);
    }
    *map = NULL;
}
