#include "node.h"


int main(int argc, char* argv[]){
    int id;
    int socket_fds[NUM_NODES], len;
    struct sockaddr_un node_addr, server_addr, client_addr;


    if(argc < 2){
        fprintf(stderr, "missing node id\n");
        return 1;
    }

    id = atoi(argv[1]);

    setup_keys(id); //generate private and public keys for this node

    //sockets setup
    //server socket for this node 
    socket_fds[id] = socket(AF_UNIX, SOCK_STREAM, 0); 
    node_addr.sun_family = AF_UNIX;
    sprintf(node_addr.sun_path, "sockets/node_%d", id);
    unlink(node_addr.sun_path);
    bind(socket_fds[id], (struct sockaddr *) &node_addr, sizeof(struct sockaddr_un));
    listen(socket_fds[id], 5);


    len = sizeof(client_addr);

    // we need exactly one connection between each pair of nodes
    for(int i = 0; i < NUM_NODES; i ++){
        if(i < id){ //connect to all nodes before me using a client socket
            socket_fds[i] = socket(AF_UNIX, SOCK_STREAM, 0);
            server_addr.sun_family = AF_UNIX;
            sprintf(server_addr.sun_path, "sockets/node_%d", i);
            
            // try connecting to node i socket
            while(connect(socket_fds[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
                sleep(1);
            }

            printf("node %d connected to node %d\n", id, i);

        }else if(i > id){ //accept all nodes after me using the server socket
            socket_fds[i] = accept(socket_fds[id], (struct sockaddr *) &client_addr, &len);

            if(socket_fds[i] == -1){
                perror("a node failed to accept");
                exit(1);
            }

        }

    }

    close(socket_fds[id]);//stop listening for more clients

    printf("node %d established all its connections\n", id);


    


}




void setup_keys(int id){
    char cmd[128], public_key_path[128], private_key_path[128];


    
    //printf("%d %s\n", id, argv[0]);

    //create directory for this node's keys
    sprintf(cmd, "mkdir -p keys/node_%d", id);
    if(system(cmd) == -1){
        fprintf(stderr, "couldn't make directory for node %d", id);
        exit(1);
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
