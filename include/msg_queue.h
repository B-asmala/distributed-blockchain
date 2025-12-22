#ifndef MSG_QUEUE
#define MSG_QUEUE

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/eventfd.h>

typedef struct Msg {
    size_t len;
    uint8_t data[2048];
    struct Msg *next;
} Msg;

typedef struct {
    Msg *head;
    Msg *tail;
    pthread_mutex_t lock;
    int wake_fd;
} MsgQueue;

static MsgQueue msg_queue;

void init_msg_queue();
int enqueue_to_msg_queue(Msg * msg);
Msg * dequeue_from_msg_queue(void);

#endif
