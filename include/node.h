#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "txpool.h"
#include "blockchain.h"
#include "keys.h"
#include "orch.h"
#include "msg_queue.h"

#define MAX_EVENTS 64

#define MAX_BUF 8192
typedef enum { READ_TYPE, READ_LEN, READ_PAYLOAD } rx_state_t;

typedef struct {
    rx_state_t state;
    uint8_t type;
    uint32_t len;

    uint8_t buff[MAX_BUF];
    size_t buff_len;
} Connection;

extern RSA ** public_keys;
extern int ID;

void setup_keys();
void load_public_keys();
void * mining_thread(void * arg);
void * transaction_generation_thread(void * arg);
void * io_thread(void * arg);
#endif
