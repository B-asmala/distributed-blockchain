#include "block.h"


atomic_bool interrupt_mining;


void calculate_merkle_root(Block * block){
    hash_t hashes[BLOCK_SIZE];
    int length = BLOCK_SIZE;
    int new_length;

    for(int i = 0; i < BLOCK_SIZE; i ++){
        memcpy(hashes[i], block->transactions[i].txid, 32);
    }

    while(length > 1){
        new_length = (length + 1) / 2;

        for(int i = 0; i < new_length; i ++){
            if(i * 2 + 1 < length){ // pair of hashes available
                hash_pair(hashes[2 * i], hashes[2 * i + 1], hashes[i]);
            }else{ // not enough hashes
                hash_pair(hashes[2 * i], hashes[2 * i], hashes[i]);
            }

        }

        length = new_length;
    }

    memcpy(block->header.merkle_root, hashes[0], HASH_SIZE);

}



void serialize_block_header(BlockHeader * blckhdr, uint8_t * buffer){
    
    memcpy(buffer, &blckhdr->nonce, 4); 
    memcpy(buffer + 4, &blckhdr->timestamp, 4); 
    memcpy(buffer + 8, blckhdr->previous_hash, HASH_SIZE); 
    memcpy(buffer + 40, blckhdr->merkle_root, HASH_SIZE); 

}

//TODO make target = number of bits not bytes
int mine_block(Block * block){
    uint8_t serialized_data[sizeof(BlockHeader)];
    hash_t hash;
    int f;
    block->header.nonce = 1;
    
    while(1){
        if(atomic_load(&interrupt_mining))return 1;

        serialize_block_header(&block->header, serialized_data);
        hash_data(serialized_data, sizeof(serialized_data), hash);

        f = 0;
        for(int i = 0; i < TARGET; i ++){
            if(hash[i] != 0){
                f = 1;
                break;
            }
        }

        if(f == 0){ //nonce found!
            memcpy(block->hash, hash, HASH_SIZE);
            return 0;

        }

        block->header.nonce ++;
        if(block->header.nonce == 0){
            return 1; //can't find nonce
        }
    }

    return 1;

}

int verify_block(Block * block, RSA ** public_keys){
    // verify transactions

    for(int i = 0; i < BLOCK_SIZE; i ++){
        if(!verify_transaction_signature(&block->transactions[i], public_keys[block->transactions[i].sender_ID]))return 0;
    }

    
    // check merkle root
    hash_t hash; 
    memcpy(hash, block->header.merkle_root, sizeof(hash_t));
    calculate_merkle_root(block); // calculate merkle root
    if(memcmp(hash, block->header.merkle_root, sizeof(hash_t)) != 0)return 0;

    // check the hash
    uint8_t serialized_data[sizeof(BlockHeader)];
    serialize_block_header(&block->header, serialized_data);
    hash_data(serialized_data, sizeof(serialized_data), hash);
    if(memcmp(hash, block->hash, sizeof(hash_t)) != 0)return 0;

    // check target
    for(int i = 0; i < TARGET; i ++){
        if(hash[i] != 0){
            return 0;
        }
    }



    return 1;
}



