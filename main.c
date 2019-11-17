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
void Superviseur();

int main() {
//    Superviseur();
    ProcessusGenerateur();

    return 0;
}
void Superviseur(){
//    Quantum = Xsecondes
    //while true
    //for sur toutes les files de msg(en priorité la 0)
    //while quantum
    //for sur la file de msg
    //start
    // mettre en pause proc courant, et à la fin de la file
    int start = 10;
    while(start > -1)
    {
        if (start != 0)
        {
            printf("Quantum n°%d\n",10-start);
            start--;
            sleep(2);
        }
        else
        {
            break;
        }
    }

}

void ProcessusGenerateur() {
    for (int i = 0; i < 5; ++i) {
        srand(time(NULL));
        sleep(1);
        if(fork() == 0) {
            processus process;
            process.pid = getpid();
            process.priorite = rand()%11;
            process.tpsExec = rand()%21;
            process.dateSoumission = rand()%6;
            printf("[Processus lancé] PID : %d,  priorite : %ld, temps d'execution : %d\n", process.pid, process.priorite, process.tpsExec);
            while(1);
            //switch priorité, += processus sur liste message du n° priorité
            //superviseur va s'occupper en priorité les prio basses. A chaque quantum de temps, il verifie si de nouveau process sont générés. Exemple:
            //superviseur execute proc de prio 3, on ajoute un proc de prio 1, il met en pause le 3 pour faire le 1
        }
    }
}


