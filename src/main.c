#include "block.h"
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>
int main(){

    // test block module

    Transaction tx;

    // Fill transaction fields
    tx.sender_ID   = 1001;
    tx.receiver_ID = 2002;
    tx.amount      = 50;
    tx.timestamp   = (uint32_t)time(NULL);

    // Compute hash (transaction ID)
    hash_transaction(&tx);

    printf("Transaction:\n");
    printf("  Sender   : %u\n", tx.sender_ID);
    printf("  Receiver : %u\n", tx.receiver_ID);
    printf("  Amount   : %u\n", tx.amount);
    printf("  Timestamp: %u\n", tx.timestamp);

    printf("  TXID     : ");
    print_hash(tx.txid);

    

    // testing merkle root 
    Block block;
    memset(&block, 0, sizeof(Block));
    char input[32];

    // Fill dummy transactions with known txid = SHA256("tx-i")
    for (int i = 0; i < BLOCK_SIZE; i++) {
        
        sprintf(input, "tx-%d", i);
        SHA256((unsigned char *)input, strlen(input), block.transactions[i].txid);
        print_hash(block.transactions[i].txid);
    }

    // Calculate and store Merkle root.
    calculate_merkle_root(&block);

    printf("merkle root : ");
    print_hash(block.header.merkle_root);
    


    


}
