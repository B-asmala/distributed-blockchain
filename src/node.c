#include "node.h"


int main(int argc, char* argv[]){
    char cmd[128], public_key_path[128], private_key_path[128];


    if(argc < 2){
        fprintf(stderr, "missing node id\n");
        return 1;
    }

    int id = atoi(argv[1]);
    //printf("%d %s\n", id, argv[0]);

    //create directory for this node's keys
    sprintf(cmd, "mkdir -p keys/node_%d", id);
    if(system(cmd) == -1){
        fprintf(stderr, "couldn't make directory for node %d", id);
        return 1;
    }
    

    sprintf(public_key_path, PUBLIC_KEY_PATH, id);
    sprintf(private_key_path, PRIVATE_KEY_PATH, id);

    // If keys don’t exist, generate them 
    FILE *fp = fopen(private_key_path, "r");
    if (!fp) {
        generate_keys(public_key_path, private_key_path);
    } else {
        fclose(fp);
    } 
    // Load keys
    RSA *rsa_priv = load_private_key(private_key_path);
    RSA *rsa_pub  = load_public_key(public_key_path);
    if (!rsa_priv || !rsa_pub) {
        fprintf(stderr, "node %d couldn't load keys\n", id);
        exit(1);
    }

    printf("node %d done generating keys\n", id);

}
