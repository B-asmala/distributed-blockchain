#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "block.h"
#include "hash_map.h"

#define CHAIN_MAX 1000



typedef struct{
    BlockNode * genesis;
    BlockNode * longest_end;
    int max_height;
    hash_map_t * block_map;
    pthread_mutex_t lock;
    int size; // current workaround to aviod mem leaks
    pthread_mutex_t file_lock;
} Blockchain;

extern Blockchain blockchain;

void init_blockchain();
int add_to_blockchain(Block * blk);
void write_longest_chain(int id);

#endif
