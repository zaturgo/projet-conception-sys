#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;

void ProcessusGenerateur();

int main() {
    ProcessusGenerateur();

    return 0;
}

void ProcessusGenerateur() {

    sleep(1);

    if(fork() == 0) {
        processus process;
        process.pid = getpid();
        process.priorite = rand()%11;
        process.tpsExec = rand()%21;
        process.dateSoumission = rand()%6;

        printf("[Processus lancé] PID : %d,  priorite : %ld, temps d'execution : %d\n", process.pid, process.priorite, process.tpsExec);

        while(1) {
            
        }
    }

    ProcessusGenerateur();
}


