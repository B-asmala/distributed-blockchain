#include "node.h"


int main(int argc, char* argv[]){
    int id;
    pthread_t io, miner, tx_gen;
    int peer_fds[NUM_NODES - 1], node_fd, len;
    struct sockaddr_un node_addr, server_addr, client_addr;
    uint8_t type; // 0 for tx, 1 for block
    Transaction tx;
    Block blk;
    RSA * public_keys[NUM_NODES];


    if(argc < 2){
        fprintf(stderr, "missing node id\n");
        return 1;
    }

    id = atoi(argv[1]);


    setup_keys(id); //generate private and public keys for this node

    //sockets setup
    node_fd = socket(AF_UNIX, SOCK_STREAM, 0); //server socket fd for this node 
    node_addr.sun_family = AF_UNIX;
    sprintf(node_addr.sun_path, "sockets/node_%d", id);
    unlink(node_addr.sun_path);
    bind(node_fd, (struct sockaddr *) &node_addr, sizeof(struct sockaddr_un));
    listen(node_fd, 5);


    len = sizeof(client_addr);

    // we need exactly one connection between each pair of nodes
    for(int i = 0; i < NUM_NODES; i ++){
        if(i < id){ //connect to all nodes before me using a client socket
            peer_fds[i] = socket(AF_UNIX, SOCK_STREAM, 0);
            server_addr.sun_family = AF_UNIX;
            sprintf(server_addr.sun_path, "sockets/node_%d", i);
            
            // try connecting to node i socket
            while(connect(peer_fds[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
                sleep(1);
            }

            printf("node %d connected to node %d\n", id, i);

        }else if(i > id){ //accept all nodes after me using the server socket
            peer_fds[i - 1] = accept(node_fd, (struct sockaddr *) &client_addr, &len);

            if(peer_fds[i - 1] == -1){
                perror("a node failed to accept");
                exit(1);
            }

        }

    }

    close(node_fd);//stop listening for more clients

    printf("node %d established all its connections\n", id);




    
    //broadcast test data, a dummy transaction for example
    type = 0;
    size_t sz;
    sz = sizeof(Transaction);
    tx.sender_ID = id;
    tx.amount = 1000000000;

    hash_transaction(&tx);

    printf("node %d broadcasted this : ", id);
    print_hash(tx.txid); 

    for(int i = 0; i < NUM_NODES; i++){
        write(peer_fds[i], &type, 1); // send type of data
        write(peer_fds[i], &sz, sizeof(uint32_t)); // send size of data
        write(peer_fds[i], &tx, sizeof(Transaction)); //send actual data
    }

    //threads for: mining, generating transactions, handling IO
    pthread_create(&miner, NULL, mining_thread, NULL);
    pthread_create(&tx_gen, NULL, transaction_generation_thread, NULL);
    pthread_create(&io, NULL, io_thread, peer_fds);
    
    pthread_join(miner, NULL);
    pthread_join(tx_gen, NULL);
    pthread_join(io, NULL);


    


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

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl()");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl()");
    }
}

void handle_payload(uint8_t type, uint32_t len, uint8_t * payload){
    if(type == 0){ //transaction
        if(len > sizeof(Transaction)){
            fprintf(stderr, "payload too big\n");
            return;
        }
        Transaction tx;
        memcpy(&tx, payload, len);

        printf("i received this from node %u : ", tx.sender_ID);
        print_hash(tx.txid);
        printf("amount : %u\n", tx.amount);

        // verify & add to txpool
    }else{ //block
        if(len > sizeof(Block)){
            fprintf(stderr, "payload too big\n");
            return;
        }
        Block blk;
        memcpy(&blk, payload, len);

        //verify & add to blockchain
    }   


}

void * mining_thread(void * arg){
    while(1){}
    return NULL;

}

void * transaction_generation_thread(void * arg){
    while(1){}
    return NULL;
}

void * io_thread(void * arg){

    int fd, *fds = (int*) arg;
    struct epoll_event event, * events;
    ssize_t nbytes;
    Connection conns[NUM_NODES + 3], * conn;
        
    //create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1()");
        exit(1);
    }


    memset(&event, 0, sizeof(event));

    // register fds in the epoll interest list
    for(int i = 0; i < NUM_NODES - 1; i ++){
        set_nonblocking(fds[i]);

        event.data.fd = fds[i];
        event.events = EPOLLIN | EPOLLET;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[i], &event) == -1){
            perror("epoll_ctl");
            exit(1);
        }
    }

    // initialize connections state
    for(int i = 0; i < NUM_NODES - 1; i ++){
        conns[i].state = READ_TYPE;
        conns[i].buff_len = 0;

    }

    events = calloc(MAX_EVENTS, sizeof(event));

    while(1){
        int nevents = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nevents == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }


        for(int i = 0; i < nevents; i ++){
            fd = events[i].data.fd;
            conn = &conns[fd];

            // Check for errors
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error on fd %d\n", fd);
                break;
            }



            //read data
            //keep reading till we encounter EWOULDBLOCK
            while(1){
                // read till the connection buffer is full 
                nbytes = read(fd, conn->buff + conn->buff_len , sizeof(conn->buff) - conn->buff_len);

                if(nbytes == -1){
                    if(errno == EAGAIN || errno == EWOULDBLOCK)break; //no more data to read
                    perror("read"); // else it is an actual error 
                    break;
                }

                if(nbytes == 0){ // socket closed by peer
                    printf("fd %d closed by peer\n", fd);
                    close(fd);
                    break;
                }


                // we actually read some data, we need to parse it
                conn->buff_len += nbytes;

                //parsing loop
                size_t read_mark = 0; // next position to parse
                while(1){
                    if(conn->state == READ_TYPE){
                        if(conn->buff_len - read_mark < sizeof(conn->type))break; // not enough bytes to read type, break and come back after reading more 
                        
                        conn->type = conn->buff[read_mark];
                        read_mark ++;
                        conn->state = READ_LEN;

                    }

                    if(conn->state == READ_LEN){
                        if(conn->buff_len - read_mark < sizeof(conn->len))break;
                        memcpy(&conn->len, conn->buff + read_mark, 4);
                        read_mark += 4;
                        conn->state = READ_PAYLOAD;
                    }


                    if(conn->state == READ_PAYLOAD){
                        if(conn->buff_len - read_mark < conn->len)break;

                        //else full payload is here, handle it
                        handle_payload(conn->type, conn->len, conn->buff + read_mark);

                        read_mark += conn->len;
                        conn->state = READ_TYPE;



                    }

                }
                
                // move the leftovers to the beginning of the buffer
                memmove(conn->buff, conn->buff + read_mark, conn->buff_len - read_mark);
                conn->buff_len -= read_mark;



                
                
            }

        }

    }
    
    free(events);
    close(epoll_fd);
    return NULL;

}
