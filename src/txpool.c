#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/sha.h>
#include <time.h>
#include "block.h"
#include "txpool.h"


TransactionPool * init_transaction_pool(){
    TransactionPool * pool = malloc(sizeof(TransactionPool));
    if(pool == NULL){
        return NULL;
    }
    pool->size = 0;
    pool->head = NULL;
    pool->tail = NULL;
    return pool;
}

int enqueue_to_transaction_pool(TransactionPool* pool, Transaction* tx){
    if(pool == NULL || tx == NULL){
        return 1;
    }

    if(pool->size >= TX_POOL_CAPACITY){
        return 1;
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


    return 0;
}

//TODO
//int remove_batch_from_transaction_pool(TransactionPool* pool, Transaction* tx){}


