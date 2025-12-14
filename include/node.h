#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "txpool.h"
#include "hash_set.h"
#include "block.h"
#include "keys.h"
#include "orch.h"

void setup_keys(int id);



#endif
