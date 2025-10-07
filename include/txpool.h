#ifndef TXPOOL_H
#define TXPOOL_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "block.h"
#include "hash_set.h"


#define TX_POOL_CAPACITY 100




typedef struct{
    Transaction * current;
    struct TransactionNode * next;
    // prev?

} TransactionNode;


typedef struct{
    TransactionNode * head;
    TransactionNode * tail;
    hash_set_t * transaction_set;
    int size;
} TransactionPool;


TransactionPool * init_transaction_pool();
int enqueue_to_transaction_pool(TransactionPool* pool, Transaction* tx);
int dequeue_from_transaction_pool(TransactionPool* pool, Transaction* tx);

#endif
