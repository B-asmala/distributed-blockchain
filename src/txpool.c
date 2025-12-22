#include "txpool.h"

void init_transaction_pool(){
    tx_pool.head = tx_pool.tail = NULL;
    pthread_mutex_init(&tx_pool.lock, NULL);
}

int enqueue_to_transaction_pool(Transaction* tx){
    if(tx == NULL){
        return 1;
    }

    TransactionNode * txn = malloc(sizeof(TransactionNode));

    pthread_mutex_lock(&tx_pool.lock); 
    txn->current = tx;
    txn->next = NULL;

    if(tx_pool.tail == NULL){
        tx_pool.head = txn;
    }else{
        tx_pool.tail->next = txn;
    }

    tx_pool.tail = txn;
    pthread_mutex_unlock(&tx_pool.lock);


    return 0;
}

Transaction * dequeue_from_transaction_pool(){
    if(tx_pool.head == NULL)return NULL;

    TransactionNode * txn = tx_pool.head;
    Transaction * tx = txn->current;


    if(tx_pool.head->next == NULL){
        tx_pool.tail = NULL;
    }

    tx_pool.head = tx_pool.head->next;
    
    free(txn);
    
    return tx;
}

Transaction ** dequeue_batch_from_transaction_pool(){
    
    Transaction ** tx_arr = malloc(BLOCK_SIZE * sizeof(Transaction *));
    Transaction * tx = NULL;

    for(int i = 0; i < BLOCK_SIZE; i ++){
        while(tx == NULL){
            pthread_mutex_lock(&tx_pool.lock);
            tx = dequeue_from_transaction_pool();
            pthread_mutex_unlock(&tx_pool.lock);
        }

        tx_arr[i] = tx;
        tx = NULL;
    }

    return tx_arr;
}
