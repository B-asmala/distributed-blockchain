#ifndef HASH_SET_H
#define HASH_SET_H


#include <stdint.h>
#include "uthash.h"
#include "block.h"


typedef struct {
    hash_t key;
    UT_hash_handle hh;


} hash_set_t;

void hash_set_add(hash_set_t ** set, const hash_t key);
int hash_set_contains(hash_set_t ** set, const hash_t key);
void hash_set_remove(hash_set_t ** set, const hash_t key);
void hash_set_free(hash_set_t ** set);



#endif
