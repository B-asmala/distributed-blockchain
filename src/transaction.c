#include "transaction.h"

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
