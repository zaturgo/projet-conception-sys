#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NB_FILEMSG 11

int CLE_PRIORITE[11];

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;

void ProcessusGenerateur();
void Superviseur();

int main() {
    int msgid[NB_FILEMSG];

    // Création des files de message
    for(int i=0; i<NB_FILEMSG; i++) {
        CLE_PRIORITE[i] = i;
        if ((msgid[i] = msgget(CLE_PRIORITE[i], 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 0\n");
        exit(1);
        }
    }
    

//    Superviseur();
    ProcessusGenerateur();

    // Suppression des files de messages
    for(int i=0; i<NB_FILEMSG; i++) {
        msgctl(msgid[i], IPC_RMID, NULL);
    }

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

// Générateur de processus aléatoire
void ProcessusGenerateur() {
    for (int i = 0; i < 5; ++i) {
        srand(time(NULL));
        sleep(0.5);
        if(fork() == 0) {
            processus process;
            process.pid = getpid();
            process.priorite = rand()%11;
            process.tpsExec = rand()%21;
            process.dateSoumission = rand()%6;
            printf("[Processus lancé] PID : %d,  priorite : %ld, temps d'execution : %d\n", process.pid, process.priorite, process.tpsExec);
            sleep(10);
            exit(0);//kill après 10 secondes
            //switch priorité, += processus sur liste message du n° priorité
            //superviseur va s'occupper en priorité les prio basses. A chaque quantum de temps, il verifie si de nouveau process sont générés. Exemple:
            //superviseur execute proc de prio 3, on ajoute un proc de prio 1, il met en pause le 3 pour faire le 1
        }
    }

    //ProcessusGenerateur();
}

