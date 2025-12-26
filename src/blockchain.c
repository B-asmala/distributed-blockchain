#include "blockchain.h"

Blockchain blockchain;

void init_blockchain(){
    blockchain.genesis = calloc(1, sizeof(BlockNode)); // all zeros
    hash_map_put(&blockchain.block_map, blockchain.genesis->current.hash, blockchain.genesis);
    blockchain.longest_end = blockchain.genesis;
    blockchain.size = 0;
    blockchain.max_height = 0;
    pthread_mutex_init(&blockchain.lock, NULL);
    pthread_mutex_init(&blockchain.file_lock, NULL);
}


int add_to_blockchain(Block * blk){
    pthread_mutex_lock(&blockchain.lock);

    if(hash_map_get(&blockchain.block_map, blk->hash)){
        pthread_mutex_unlock(&blockchain.lock);
        return 1; //already exists
    }

    BlockNode * prev = hash_map_get(&blockchain.block_map, blk->header.previous_hash);
    if(prev == NULL){
        pthread_mutex_unlock(&blockchain.lock);
        return 1; // i don't have the previous block
    }

    BlockNode * new = malloc(sizeof(BlockNode));
    new->current = *blk;
    new->prev = prev;
    new->height = prev->height + 1;

    hash_map_put(&blockchain.block_map, new->current.hash, new);

    if(new->height > blockchain.max_height){
        blockchain.max_height = new->height;
        blockchain.longest_end = new;
    }

    pthread_mutex_unlock(&blockchain.lock);
    return 0;

}

void write_longest_chain(int id){
    //printf("node %d has %d blocks\n", id, blockchain.max_height);
    
    hash_t hash;
    BlockNode * blkn;
    FILE * f;
    char file_path[128];

    pthread_mutex_lock(&blockchain.lock);
    blkn = blockchain.longest_end;
    pthread_mutex_unlock(&blockchain.lock);
    
    memset(hash, 0, sizeof(hash_t));
    
    sprintf(file_path, "./blockchains/node_%d", id);
    f = fopen(file_path, "w");

    while(memcmp(hash, blkn->current.hash, sizeof(hash_t)) != 0){
        
        pthread_mutex_lock(&blockchain.file_lock);
        fprintf(f, "[block %d : ", blkn->height);
        for (size_t i = 0; i < HASH_SIZE; i++)
            fprintf(f, "%02x", blkn->current.hash[i]);
        fprintf(f, " ]\n");
        fprintf(f, "                        |\n");
        fprintf(f, "                        V\n");
        pthread_mutex_unlock(&blockchain.file_lock);



        blkn = blkn->prev;

    }

    pthread_mutex_lock(&blockchain.file_lock);
    fprintf(f, "                     genesis\n");
    pthread_mutex_unlock(&blockchain.file_lock);


    fclose(f);


}

