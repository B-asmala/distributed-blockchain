#include "msg_queue.h"

MsgQueue msg_queue;

void init_msg_queue(){

    msg_queue.head = msg_queue.tail = NULL;
    pthread_mutex_init(&msg_queue.lock, NULL);
}


int enqueue_to_msg_queue(Msg * msg){
    if(msg == NULL)return 1;

    pthread_mutex_lock(&msg_queue.lock);
    msg->next = NULL;

    if(msg_queue.tail == NULL){
        msg_queue.head = msg;
    }else{
        msg_queue.tail->next = msg;
    }
    msg_queue.tail = msg;
    
    pthread_mutex_unlock(&msg_queue.lock);

    return 0;
}

Msg * dequeue_from_msg_queue(void){ // gets current head only (advancing done after broadcasting)

    pthread_mutex_lock(&msg_queue.lock);
    if(msg_queue.head == NULL){
        
        pthread_mutex_unlock(&msg_queue.lock);
        return NULL;
    }

    Msg * msg = msg_queue.head;

    //if(msg_queue.head->next == NULL){
        //msg_queue.tail = NULL;
    //}

    //msg_queue.head = msg_queue.head->next;
    

    pthread_mutex_unlock(&msg_queue.lock);
    return msg;

    
}


