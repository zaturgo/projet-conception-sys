#include <stdio.h>
#include <unistd.h>
<<<<<<< Updated upstream
#include <stdlib.h>
=======
#include <time.h>
>>>>>>> Stashed changes

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
    if(fork() == 0) {
<<<<<<< Updated upstream

=======
>>>>>>> Stashed changes
        processus process;
        process.pid = getpid();
        process.priorite = rand()%11;
        process.tpsExec = rand()%21;
        process.dateSoumission = rand()%6;

        printf("[Processus lancé] PID : %ld  priorite", process.priorite);

        while(1) {

        }
    }
}


