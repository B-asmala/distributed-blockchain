#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdlib.h>
#include <stdint.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/bn.h>

#define HASH_SIZE 32

typedef uint8_t hash_t[HASH_SIZE];

typedef struct{
    hash_t txid;
    uint32_t sender_ID;
    uint32_t receiver_ID;
    uint32_t amount;
    uint32_t timestamp;
    uint8_t signature[256];
} Transaction; 

void hash_data(const uint8_t * data, size_t len, hash_t hashed_data);
void hash_string(const char * str, hash_t  hashed_str);
void print_hash(hash_t hash);
void hash_transaction(Transaction * tx);
void hash_pair(const hash_t first, const hash_t second, hash_t result);
void sign_data(const uint8_t * data, size_t len, uint8_t * sig, uint32_t * sig_len, RSA * rsa_priv);
int verify_data_signature(const uint8_t *data, size_t len, const uint8_t * sig, uint32_t sig_len, RSA * rsa_pub);
void sign_transaction(Transaction * tx, RSA * rsa_priv);
int verify_transaction_signature(Transaction * tx, RSA * rsa_pub);


#endif
