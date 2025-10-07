#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/sha.h>
#include <time.h>
#include "block.h"
#include "txpool.h"
#include "hash_set.h"

TransactionPool * init_transaction_pool(){
    TransactionPool * pool = malloc(sizeof(TransactionPool));
    if(pool == NULL){
        return NULL;
    }
    pool->size = 0;
    pool->head = NULL;
    pool->tail = NULL;
    pool->transaction_set = NULL;
    return pool;
}

int enqueue_to_transaction_pool(TransactionPool* pool, Transaction* tx){
    if(pool == NULL || tx == NULL){
        return 1;
    }

    if(pool->size >= TX_POOL_CAPACITY){
        return 1;
    }

    if(hash_set_contains(&(pool->transaction_set), tx->txid)){
        return 1; //might need to change this
    }

    TransactionNode * txn = malloc(sizeof(TransactionNode));

    txn->current = tx;
    txn->next = NULL;

    if(pool->tail){
        pool->tail->next = txn;

    }else{
        pool->head = txn;
    }

    pool->tail = txn;
    pool->size ++;
    hash_set_add(&(pool->transaction_set), tx->txid);

    return 0;
}

int dequeue_from_transaction_pool(TransactionPool * pool, Transaction * tx){
    if(pool == NULL || tx == NULL){
        return 1;
    }

    if(pool->size <= 0){
        return 1;
    }

    
    TransactionNode * txn = pool->head;
    pool->head = txn->next;

    if(pool->head == NULL){
        pool->tail = NULL;
    }
    pool->size --;


    *tx = *(txn->current);
    free(txn);
    
    //might not need to immediately remove it from the hash table
    //hash_set_remove(&(pool->transaction_set), tx->txid);



    return 0;
}

//TODO
//int remove_batch_from_transaction_pool(TransactionPool* pool, Transaction* tx){}
//transaction_pool_free

