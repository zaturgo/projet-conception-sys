#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CLE 1

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;

void ProcessusGenerateur();
void Superviseur();

int main() {
    int msgid;

    // Création de la file de messages
    if ((msgid = msgget(CLE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file 0\n");
        exit(1);
    }
    

//    Superviseur();
    ProcessusGenerateur();

    // Suppression de la file de messages
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
void Superviseur() {
//    Quantum = Xsecondes
//var globale proc
    int start = 5;
    while (start > -1) {//tant qu'il y a des processus
        if (start != 0) {
            printf("Quantum n°%d\n", 5 - start);
            //for sur les files de message ->i
              //si elle contient un processus
              //this.proc = getProcFileMsg();prend le premier processus de la file
              //timer = true;
              //timer 2 sec {timer = false; this.proc.pause();}
              //while(timer)
                    //resume this.proc
                    //wait(this.proc);
                    //this.proc = getProcFileMsg();prend le premier processus de la file
            // }

                //quand proc fini, get procFileMsg et recommencer
                // si execution > 2secondes, mettre en pause proc actuel, et renvoyer dans la file msg
            start--;
            sleep(2);
        } else {
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
        }
    }
}

