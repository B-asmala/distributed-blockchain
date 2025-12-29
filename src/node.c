#include "node.h"


RSA ** public_keys;
int ID;


int main(int argc, char* argv[]){
    pthread_t io, miner, tx_gen;
    int peer_fds[NUM_NODES - 1], node_fd;
    struct sockaddr_un node_addr, server_addr, client_addr;
    socklen_t len; 
    // seed rand
    srand(time(NULL));


    // INITS
    // get id of this node
    if(argc < 2){
        fprintf(stderr, "missing node id\n");
        return 1;
    }
    ID = atoi(argv[1]); 

    //generate private and public keys for this node
    setup_keys(ID);     
    

    //sockets setup
    node_fd = socket(AF_UNIX, SOCK_STREAM, 0); //server socket fd for this node 
    node_addr.sun_family = AF_UNIX;
    sprintf(node_addr.sun_path, "sockets/node_%d", ID);
    unlink(node_addr.sun_path);
    if (bind(node_fd, (struct sockaddr *)&node_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind failed");
        close(node_fd);   
        exit(EXIT_FAILURE); 
    }
    listen(node_fd, 5); // start listening for clients


    len = sizeof(client_addr);

    // establish mesh network
    // we need exactly one connection between each pair of nodes
    for(int i = 0; i < NUM_NODES; i ++){
        if(i < ID){ //connect to all nodes before me using a client socket
            peer_fds[i] = socket(AF_UNIX, SOCK_STREAM, 0);
            server_addr.sun_family = AF_UNIX;
            sprintf(server_addr.sun_path, "sockets/node_%d", i);
            
            // try connecting to node i socket
            while(connect(peer_fds[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
                sleep(1);
            }

            printf("node %d connected to node %d\n", ID, i);

        }else if(i > ID){ //accept all nodes after me using the server socket
            peer_fds[i - 1] = accept(node_fd, (struct sockaddr *) &client_addr, &len);

            if(peer_fds[i - 1] == -1){
                perror("a node failed to accept");
                exit(1);
            }

        }

    }

    close(node_fd);//stop listening for more clients

    printf("node %d established all its connections\n", ID);


    // init message queue
    init_msg_queue();
    msg_queue.wake_fd = eventfd(0, EFD_NONBLOCK);
    if (msg_queue.wake_fd == -1) {
        perror("eventfd");
        exit(1);
    }

    // init transaction pool
    init_transaction_pool(); 

    //load public keys
    load_public_keys();

    // init blockchain
    init_blockchain();
    
    //threads for: mining, generating transactions, handling IO
    pthread_create(&miner, NULL, mining_thread, NULL);
    pthread_create(&tx_gen, NULL, transaction_generation_thread, NULL);
    pthread_create(&io, NULL, io_thread, peer_fds);
    
    pthread_join(miner, NULL);
    pthread_join(tx_gen, NULL);
    pthread_join(io, NULL);


    


}


void setup_keys(){
    char cmd[128], public_key_path[128], private_key_path[128];


    //create directory for this node's keys
    sprintf(cmd, "mkdir -p keys/node_%d", ID);
    if(system(cmd) == -1){
        fprintf(stderr, "couldn't make directory for node %d", ID);
        exit(1);
    }
    

    sprintf(public_key_path, PUBLIC_KEY_PATH, ID);
    sprintf(private_key_path, PRIVATE_KEY_PATH, ID);

    // If keys don’t exist, generate them 
    FILE *fp = fopen(private_key_path, "r");
    if (!fp) {
        generate_keys(public_key_path, private_key_path);
    } else {
        fclose(fp);
    } 
    // try loading keys
    RSA *rsa_priv = load_private_key(private_key_path);
    RSA *rsa_pub  = load_public_key(public_key_path);
    if (!rsa_priv || !rsa_pub) {
        fprintf(stderr, "node %d couldn't load keys\n", ID);
        exit(1);
    }

    printf("node %d done generating keys\n", ID);

}

void load_public_keys(){
    public_keys = malloc(NUM_NODES * sizeof(RSA *));
    if (!public_keys) {
        perror("malloc public_keys");
        exit(1);
    }

    char public_key_path[128];
    
    for(int i = 0; i < NUM_NODES; i ++){
        sprintf(public_key_path, PUBLIC_KEY_PATH, i);
        public_keys[i] = load_public_key(public_key_path);
    }
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
        Transaction * tx = malloc(sizeof(Transaction));
        memcpy(tx, payload, len);


        // verify & add to txpool
        if(verify_transaction_signature(tx, public_keys[tx->sender_ID])){
            enqueue_to_transaction_pool(tx);
        }else{
            printf(">>>>>>>>>>>>> i received a scam transaction from %d \n", tx->sender_ID);
            free(tx);
        }
                

    }else{ //block
        if(len > sizeof(Block)){
            fprintf(stderr, "payload too big\n");
            return;
        }

        Block * blk = malloc(sizeof(Block));
        memcpy(blk, payload, len);


        //verify & add to blockchain
        if(verify_block(blk, public_keys)){
            if(add_to_blockchain(blk) == 0){ // block added successfully
                atomic_store(&interrupt_mining, true);
                write_longest_chain(ID);
            }
        }else{
            printf(">>>>>>>>>>>>> i received a scam block with hash : ");
            print_hash(blk->hash);
        }


        free(blk);
        

    }   


}

void * mining_thread(void * arg){
    Transaction ** tx_arr;
    Block * blk = NULL;
    blk = malloc(sizeof(Block));
    int type = 1;
    uint32_t sz = sizeof(Block);
    uint64_t one = 1;
    Msg * msg;



    if (!blk) {
        perror("malloc Block");
        exit(1);
    }

    while(1){
        atomic_store(&interrupt_mining, false);
        blk->header.timestamp = (uint32_t)time(NULL);
        memcpy(blk->header.previous_hash, blockchain.longest_end->current.hash, sizeof(hash_t));
        tx_arr = dequeue_batch_from_transaction_pool();
        for(int i = 0; i < BLOCK_SIZE; i ++){
            blk->transactions[i] = *tx_arr[i];
            free(tx_arr[i]);
        }
        free(tx_arr);

        calculate_merkle_root(blk);

        if(mine_block(blk) == 0){ // success
            add_to_blockchain(blk);
            printf("node %d just mined a block! \n", ID);
            write_longest_chain(ID);
            

            msg = malloc(sizeof(Msg));
            msg->len = 1 + sizeof(sz) + sizeof(Block);
            memcpy(msg->data, &type, 1);
            memcpy(msg->data + 1, &sz, sizeof(sz));
            memcpy(msg->data + 1 + sizeof(sz), blk, sizeof(Block));
            msg->rem_writes = NUM_NODES - 1;

            enqueue_to_msg_queue(msg);

            //signal for io thread to broadcast
            write(msg_queue.wake_fd, &one, sizeof(one));
            continue;



        }

        //printf("node %d iterrupted while mining \n", ID);
    }
    return NULL;

}

void * transaction_generation_thread(void * arg){
    char private_key_path[128];
    RSA * rsa_priv;
    int rec_id, type = 0;
    Transaction * tx= NULL;
    uint32_t sz = sizeof(Transaction);
    uint64_t one = 1;
    Msg * msg;

    sprintf(private_key_path, PRIVATE_KEY_PATH, ID);
    rsa_priv = load_private_key(private_key_path);

    while(1){
        // generate a new tx every random period
        sleep(rand() % 10);

        // random reciever 
        do {
            rec_id = rand() % NUM_NODES;
        }while(rec_id == ID);

        tx = malloc(sizeof(Transaction));
        tx->sender_ID = ID;
        tx->receiver_ID = rec_id;
        tx->amount = rand() % 1000000;
        tx->timestamp = (uint32_t)time(NULL);

        hash_transaction(tx);
        sign_transaction(tx, rsa_priv);


        //add to your own tx pool
        enqueue_to_transaction_pool(tx);

        //serialize tx ond add to msg q to be broadcasted to other nodes
        msg = malloc(sizeof(Msg));
        msg->len = 1 + sizeof(sz) + sizeof(Transaction);
        memcpy(msg->data, &type, 1);
        memcpy(msg->data + 1, &sz, sizeof(sz));
        memcpy(msg->data + 1 + sizeof(sz), tx, sizeof(Transaction));
        msg->rem_writes = NUM_NODES - 1;

        enqueue_to_msg_queue(msg);

        //signal for io thread to broadcast
        write(msg_queue.wake_fd, &one, sizeof(one));
        printf("node %d made a transaction : ", ID);
        print_hash(tx->txid);

        
    }
    return NULL;
}

void * io_thread(void * arg){

    int fd, *fds = (int*) arg, wfd = msg_queue.wake_fd;
    Connection conns[NUM_NODES + 10], * conn;
    struct epoll_event event, * events;


        
    //create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1()");
        exit(1);
    }


    memset(&event, 0, sizeof(event));

    // register message queue wake fd
    set_nonblocking(wfd);
    event.events = EPOLLIN;
    event.data.fd = wfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wfd, &event) == -1) {
        perror("epoll_ctl wake_fd");
        exit(1);
    }

    // register peer fds in the epoll interest list
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
        conns[fds[i]].state = READ_TYPE;
        conns[fds[i]].inbuff_len = 0;
        conns[fds[i]].current_msg = NULL;
        conns[fds[i]].written_len = 0;
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
            if (events[i].events & (EPOLLERR | EPOLLHUP)){
                fprintf(stderr, "epoll error on fd %d\n", fd);
                break;
            }

            
            // new message to broadcast 
            if(fd == wfd){
                //drain wake up 
                while(1){
                    uint64_t x;
                    ssize_t nbytes = read(wfd, &x, sizeof(x));

                    if(nbytes == -1){
                        if(errno == EAGAIN || errno == EWOULDBLOCK)break; //fully drained
                        perror("read"); // else it is an actual error 
                        break;
                    }
                }

                
                // update connections msg pointers if not already set                
                Msg * msg = dequeue_from_msg_queue();
                for(int i = 0; i < NUM_NODES - 1; i ++){
                    if(conns[fds[i]].current_msg == NULL){
                        conns[fds[i]].current_msg = msg;
                        conns[fds[i]].written_len = 0;
                    }
                }



                //enable EPOLLOUT : notifies me when fd becomes writable
                struct epoll_event ev;
                for (int i = 0; i < NUM_NODES - 1; i++) {
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                    ev.data.fd = fds[i];
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fds[i], &ev) == -1){
                        perror("epoll_ctl");
                        exit(1);
                    }
                }
            }


            //write if EPOLLOUT is enabled 
            if(events[i].events & EPOLLOUT){
                while(1){

                    ssize_t n = write(fd, conn->current_msg->data + conn->written_len, conn->current_msg->len - conn->written_len);

                    if(n > 0){
                        conn->written_len += n;
                    }else if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
                        break;
                    }else{
                        perror("write");
                        exit(1);
                    }


                    pthread_mutex_lock(&msg_queue.lock);
                    if(conn->written_len == conn->current_msg->len){
                        conn->written_len = 0;
                        Msg * next_msg = conn->current_msg->next;
                        
                        conn->current_msg->rem_writes --;
                        if(conn->current_msg->rem_writes == 0){
                            free(conn->current_msg);
                        }
                        conn->current_msg = next_msg;
                        
                    }
                    pthread_mutex_unlock(&msg_queue.lock);



                    if(conn->current_msg == NULL){ // all message queue done for this fd
                        struct epoll_event ev;
                        ev.events = EPOLLIN | EPOLLET;
                        ev.data.fd = fd;
                        if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1){
                            perror("epoll_ctl");
                            exit(1);
                        }
                        break;
                    }

                }
                

            }



            //read data
            //keep reading till we encounter EWOULDBLOCK
            while(1){
                // read till the connection buffer is full 
                ssize_t nbytes = read(fd, conn->inbuff + conn->inbuff_len , sizeof(conn->inbuff) - conn->inbuff_len);

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
                conn->inbuff_len += nbytes;

                //parsing loop
                size_t parsed_mark = 0; // next position to parse
                while(1){
                    if(conn->state == READ_TYPE){
                        if(conn->inbuff_len - parsed_mark < sizeof(conn->type))break; // not enough bytes to read type, break and come back after reading more 
                        
                        conn->type = conn->inbuff[parsed_mark];
                        parsed_mark ++;
                        conn->state = READ_LEN;

                    }

                    if(conn->state == READ_LEN){
                        if(conn->inbuff_len - parsed_mark < sizeof(conn->len))break;
                        memcpy(&conn->len, conn->inbuff + parsed_mark, 4);
                        parsed_mark += 4;
                        conn->state = READ_PAYLOAD;
                    }


                    if(conn->state == READ_PAYLOAD){
                        if(conn->inbuff_len - parsed_mark < conn->len)break;

                        //else full payload is here, handle it
                        handle_payload(conn->type, conn->len, conn->inbuff + parsed_mark);

                        parsed_mark += conn->len;
                        conn->state = READ_TYPE;



                    }

                }
                
                // move the leftovers to the beginning of the buffer
                memmove(conn->inbuff, conn->inbuff + parsed_mark, conn->inbuff_len - parsed_mark);
                conn->inbuff_len -= parsed_mark;

                
            }

        }

    }
    
    free(events);
    close(epoll_fd);
    return NULL;

}
