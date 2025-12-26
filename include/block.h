#ifndef Block_H
#define Block_H

#include "stdatomic.h"
#include "transaction.h" 
#include "unistd.h"

#define BLOCK_SIZE 5
#define TARGET 3




typedef struct {
    uint32_t nonce;
    uint32_t timestamp; 
    hash_t previous_hash;
    hash_t merkle_root;

} BlockHeader;

typedef struct{
    BlockHeader header;
    Transaction transactions[BLOCK_SIZE];
    hash_t hash;
} Block;

typedef struct BlockNode{
    Block current;
    struct BlockNode * prev;
    int height;
} BlockNode;

extern atomic_bool interrupt_mining;

void calculate_merkle_root(Block * block);
void serialize_block_header(BlockHeader * blckhdr, uint8_t * buffer);
int mine_block(Block * block);
int verify_block(Block * block, RSA ** public_keys);
#endif
