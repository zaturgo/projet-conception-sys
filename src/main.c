#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>

#define CLE 123

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;


void ProcessusGenerateur(int, int);
void Superviseur(int);



int main() {
    int msgid;
    int nbProcess;

    // Création de la file de messages
    if ((msgid = msgget(CLE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file\n");
        exit(1);
    }

    printf("Entrez le nombre de processus : ");
    scanf("%d", &nbProcess);
    

    ProcessusGenerateur(msgid, nbProcess);
    sleep(1);
    Superviseur(msgid);

    sleep(1);
    // Suppression de la file de messages
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}



void Superviseur(int msgid) {
//    Quantum = Xsecondes
//var globale proc
    //int start = 5;
    int quantum = 0;
    bool fileVide = false;
    processus process;

    printf("\n- LANCEMENT DU SUPERVISEUR\n");
    // Tant que la file n'est pas vide
    while(!fileVide) {
        fileVide = true;
        // Parcourt les priorité
        for(int i=1; i<12; i++) {
            // Récupère le 1er processus de priorité i s'il existe
            if ((msgrcv(msgid, &process, sizeof(processus) - 4, i, IPC_NOWAIT)) == -1) {
//                printf("La priorité %d n'est pas dans la file !\n", i);
            } else {
                fileVide = false;
                //sleep(1);
                if(process.tpsExec >= 1) {
                    process.tpsExec -= 1;
                    printf("[PROCESSUS LU]      PID : %d,  priorite : %ld, temps d'execution : %d, date de soumissions : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);

                    if (msgsnd(msgid, &process, sizeof(processus) - 4, 0) == -1) {
                        perror("Erreur de lecture requete \n");
                        exit(1);
                    }
                    break;
                }
            }
        }
    }


    /*while (start > -1) {//tant qu'il y a des processus
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
    }*/
}



// Générateur de processus aléatoire
void ProcessusGenerateur(int msgid, int nbProcess) {
    printf("\n- LANCEMENT DU GENERATEUR DE PROCESSUS\n");
    for (int i = 0; i < nbProcess; ++i) {
        //sleep(1);
        if(fork() == 0) {
            /*int msgid;
            if ((msgid = msgget(CLE, 0750)) == -1) {
                perror("Erreur la file n'existe pas\n");
                exit(1);
            }*/
            srand(getpid());
            processus process;
            process.pid = getpid();
            process.priorite = rand()%11 +1;
            process.tpsExec = rand()%5 +1;
            process.dateSoumission = rand()%21;
            printf("[PROCESSUS LANCE]   PID : %d,  priorite : %ld, temps d'execution : %d, date de soumissions : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);


            // Envoi du processus dans la file de message
            if (msgsnd(msgid, &process, sizeof(processus) - 4,0) == -1) {
                perror("Erreur de lecture requete \n");
                exit(1);
            }
            //sleep(10);
            exit(0);//kill après 10 secondes
        }
    }
}
