#include "orch.h"

int main() {
    char idstr[9], nodestr[7];

    if(system("mkdir -p keys") == -1){
        fprintf(stderr, "couldn't make keys directoy\n");
        return 1;
    }
    
    for (int i = 0; i < NUM_NODES; i++) {
        pid_t pid = fork();
        if (pid == 0) { // each node 
            sprintf(idstr, "%d", i);
            sprintf(nodestr, "node%d", i);
            execl("./bin/node", nodestr, idstr, NULL);
            perror("execl failed");
            exit(1);
        }
    }
    for (int i = 0; i < NUM_NODES; i++) wait(NULL);
}

