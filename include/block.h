#ifndef Block_H
#define Block_H

#include "transaction.h" 

#define BLOCK_SIZE 5
#define TARGET 3




typedef struct {
    uint32_t index;
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


void calculate_merkle_root(Block * block);
void serialize_block_header(BlockHeader * blckhdr, uint8_t * buffer);
int mine_block(Block * block);

#endif
