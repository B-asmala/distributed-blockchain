#include "block.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <time.h>


void hash_data(const uint8_t * data, size_t len, hash_t hashed_data){
    SHA256((const unsigned char*) data, len, hashed_data);
}


void hash_string(const char * str, hash_t hashed_str){
    hash_data((const unsigned char*)str, strlen(str), hashed_str);
}


void print_hash(hash_t hash) {
    for (int i = 0; i < HASH_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

void hash_transaction(Transaction * tx){
    //TODO: handle endianess
    //TODO transaction serializing function
    //4 uint32, 4 * 4 = 16 byte
    uint8_t serialized_data[16];
    memcpy(serialized_data, &tx->sender_ID, sizeof(uint32_t));
    memcpy(serialized_data + 4, &tx->receiver_ID, sizeof(uint32_t));
    memcpy(serialized_data + 8, &tx->amount, sizeof(uint32_t));
    memcpy(serialized_data + 12, &tx->timestamp, sizeof(uint32_t));
    hash_data(serialized_data, 16, tx->txid); 
    
    

}

void hash_pair(const hash_t first, const hash_t second, hash_t result){
    unsigned char combined_hashes[2 * HASH_SIZE];

    memcpy(combined_hashes, first, HASH_SIZE); //first half
    memcpy(combined_hashes + HASH_SIZE, second, HASH_SIZE); //second half
    hash_data(combined_hashes, 2 * HASH_SIZE, result);
}

void calculate_merkle_root(Block * block){
    hash_t hashes[BLOCK_SIZE];
    int length = BLOCK_SIZE;
    int new_length;

    for(int i = 0; i < BLOCK_SIZE; i ++){
        memcpy(hashes[i], block->transactions[i].txid, 32);
    }

    while(length > 1){
        new_length = (length + 1) / 2;

        for(int i = 0; i < new_length; i ++){
            if(i * 2 + 1 < length){ // pair of hashes available
                hash_pair(hashes[2 * i], hashes[2 * i + 1], hashes[i]);
            }else{ // not enough hashes
                hash_pair(hashes[2 * i], hashes[2 * i], hashes[i]);
            }

        }

        length = new_length;
    }

    memcpy(block->header.merkle_root, hashes[0], HASH_SIZE);



}

//TODO: update to EVP API
//general functions for signing/verifying
void sign_data(const uint8_t * data, size_t len, uint8_t * sig, uint32_t * sig_len, RSA *rsa_priv) {
    hash_t hash;
    hash_data(data, len, hash);

    RSA_sign(NID_sha256, hash, HASH_SIZE, sig, sig_len, rsa_priv);

}

int verify_data_signature(const uint8_t *data, size_t len, const uint8_t * sig, uint32_t sig_len, RSA * rsa_pub) {
    hash_t hash;
    SHA256(data, len, hash);

    return RSA_verify(NID_sha256, hash, HASH_SIZE, sig, sig_len, rsa_pub);
}

//singing/verifying transactions
void sign_transaction(Transaction * tx, RSA * rsa_priv) {
    uint32_t dummy;
    sign_data(tx->txid, HASH_SIZE, tx->signature, &dummy, rsa_priv);
}

int verify_transaction_signature(Transaction * tx, RSA * rsa_pub) {
    
    return verify_data_signature(tx->txid, HASH_SIZE, tx->signature, RSA_size(rsa_pub), rsa_pub);
    
}

void serialize_block_header(BlockHeader * blckhdr, uint8_t * buffer){
    
    //TODO handle little endian
    memcpy(buffer, &blckhdr->index, 4);
    memcpy(buffer + 4, &blckhdr->nonce, 4); 
    memcpy(buffer + 8, &blckhdr->timestamp, 4); 
    memcpy(buffer + 12, blckhdr->previous_hash, HASH_SIZE); 
    memcpy(buffer + 44, blckhdr->merkle_root, HASH_SIZE); 

}

//TODO make target = number of bits not bytes
int mine_block(Block * block){
    uint8_t serialized_data[sizeof(BlockHeader)];
    hash_t hash;
    int f;
    block->header.nonce = 1;
    
    while(1){
        serialize_block_header(&block->header, serialized_data);
        hash_data(serialized_data, sizeof(serialized_data), hash);

        f = 0;
        for(int i = 0; i < TARGET; i ++){
            if(hash[i] != 0){
                f = 1;
                break;
            }
        }

        if(f == 0){ //nonce found!
            memcpy(block->hash, hash, HASH_SIZE);
            return 0;

        }

        block->header.nonce ++;
        if(block->header.nonce == 0){
            return 1; //can't find nonce
        }
    }

    return 1;



}


