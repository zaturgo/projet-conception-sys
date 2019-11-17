#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CLE_PRIORITE_0      0
#define CLE_PRIORITE_1      1
#define CLE_PRIORITE_2      2
#define CLE_PRIORITE_3      3
#define CLE_PRIORITE_4      4
#define CLE_PRIORITE_5      5
#define CLE_PRIORITE_6      6
#define CLE_PRIORITE_7      7
#define CLE_PRIORITE_8      8
#define CLE_PRIORITE_9      9
#define CLE_PRIORITE_10     10

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;

void ProcessusGenerateur();
//void CreationFileMessages();
void Superviseur();

int main() {
    /*int msgid;
    CreationFileMessages();
    if ((msgid = msgget(CLE_PRIORITE_0, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 0\n");
        exit(1);
    }*/
//    Superviseur();
    ProcessusGenerateur();

    //msgctl(msgid, IPC_RMID, NULL);
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

// Générateur de processus aléatoire
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

    //ProcessusGenerateur();
}

/*void CreationFileMessages() {

    if ((msgid = msgget(CLE_PRIORITE_0, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 0\n");
        exit(1);
    }

    if ((msgid = msgget(CLE_PRIORITE_1, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 1\n");
        exit(1);
    }

    if ((msgid = msgget(CLE_PRIORITE_2, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 2\n");
        exit(1);
    }
    
    if ((msgid = msgget(CLE_PRIORITE_3, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 3\n");
        exit(1);
    }

    if ((msgid = msgget(CLE_PRIORITE_4, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 4\n");
        exit(1);
    }
    
    if ((msgid = msgget(CLE_PRIORITE_5, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 5\n");
        exit(1);
    }

    if ((msgid = msgget(CLE_PRIORITE_6, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 6\n");
        exit(1);
    }
    
    if ((msgid = msgget(CLE_PRIORITE_7, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 7\n");
        exit(1);
    }

    if ((msgid = msgget(CLE_PRIORITE_8, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 8\n");
        exit(1);
    }
    
    if ((msgid = msgget(CLE_PRIORITE_9, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file9\n");
        exit(1);
    }

    if ((msgid = msgget(CLE_PRIORITE_10, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 10\n");
        exit(1);
    }
}*/

