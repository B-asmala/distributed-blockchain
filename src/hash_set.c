#include <stdlib.h>
#include "hash_set.h"
#include "uthash.h"
#include "block.h"


void hash_set_add(hash_set_t ** set, const hash_t key){
    if(hash_set_contains(set, key)) return;
    hash_set_t * item;
    item = malloc(sizeof(hash_set_t));
    memcpy(item->key, key, sizeof(hash_t));
    HASH_ADD(hh, *set, key, sizeof(hash_t), item);
}

int hash_set_contains(hash_set_t ** set, const hash_t key){
    hash_set_t * item = NULL;
    HASH_FIND(hh, *set, key, sizeof(hash_t), item);
    return item != NULL;

}

void hash_set_remove(hash_set_t ** set, const hash_t key){
    if(!hash_set_contains(set, key))return;

    hash_set_t *item = NULL;
    HASH_FIND(hh, *set, key, sizeof(hash_t), item);
    HASH_DEL(*set, item);
    free(item);

}

void hash_set_free(hash_set_t ** set){
    
    hash_set_t *current, *tmp;

    HASH_ITER(hh, *set, current, tmp){
        HASH_DEL(*set, current);
        free(current);

    }

}
