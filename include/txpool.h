#ifndef TXPOOL_H
#define TXPOOL_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "block.h"

typedef struct TransactionNode{
    Transaction * current;
    struct TransactionNode * next;

} TransactionNode;

typedef struct {
    TransactionNode *head;
    TransactionNode *tail;
    pthread_mutex_t lock;
} TransactionPool;

static TransactionPool tx_pool;


void init_transaction_pool();
int enqueue_to_transaction_pool(Transaction* tx);
Transaction * dequeue_from_transaction_pool();
Transaction ** dequeue_batch_from_transaction_pool();
#endif
