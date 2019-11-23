#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>

#define CLE 123
#define PRIO_MIN  1
#define PRIO_MAX  10

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;


void ProcessusGenerateur(int, int);
void Superviseur(int, int*, int, int);


int main() {
    int msgid;
    int nbProcess;
    int dureeQuantum;
    int* tabCPU;
    int tempTabCPU[200];
    int tailleTabCPU = 0;
    char chaine[5];


    //======= Données entrées par l'utilisateur ======
    printf("Entrez le nombre de processus : ");
    scanf("%d", &nbProcess);

    printf("Entrez la durée d'un quantum : ");
    scanf("%d", &dureeQuantum);
    //================================================


    //============== Ouverture du fichier csv ===================
    FILE* fichier = fopen("../TableCPU.csv", "r");
    if (fichier==NULL) {
        printf("Ouverture fichier impossible !");
        exit(1);
    }//==========================================================


    //============= Lecture du fichier csv ======================
    // on lit une ligne après l'autre jusqu'Ã  la fin du fichier
    while (fgets(chaine, 5, fichier) != NULL) {
        tempTabCPU[tailleTabCPU] = atoi(chaine);
        tailleTabCPU++;
    }
    fclose(fichier);
    //===========================================================


    //====== Remplissage table d'allocation CPU ======
    tabCPU = malloc(tailleTabCPU * sizeof(int));

    for(int i=0; i<tailleTabCPU; i++) {
        tabCPU[i] = tempTabCPU[i];
    }//===============================================


    //================ Création de la file de messages======================
    if ((msgid = msgget(CLE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file\n");
        exit(1);
    }//=====================================================================
    

    //======= Génerateur de processus et superviseur =======
    ProcessusGenerateur(msgid, nbProcess);
    Superviseur(msgid, tabCPU, tailleTabCPU, dureeQuantum);
    //======================================================



    //=== Suppr file de msg et libération mémoire ===
    msgctl(msgid, IPC_RMID, NULL);
    free(tabCPU);
    //===============================================

    return 0;
}



void Superviseur(int msgid, int* tabCPU, int tailleTabCPU, int dureeQuantum) {
    int quantum = 0;
    bool fileVide = false;

    printf("\n\n=====================================   LANCEMENT DU SUPERVISEUR   =========================================\n");
    printf("Durée quantum : %d\n", dureeQuantum);

    // Tant que la file n'est pas vide
    while(!fileVide) {
        processus process;

        int noPriorite = tabCPU[quantum % tailleTabCPU]; // priorité cherchée
        int i = noPriorite;
        bool pasDeProcess = false; // true = pas de processus de la priorité recherchée jusqu'à la priorité MAX
        fileVide = true;

        // i = priorité | si pas de processus de prio i, alors chercher prio i+1 jusqu'a MAX
        // Si i = MAX et toujours pas de processus on recommence à la prio 0 jusqu'au noPriorite recherchée
        while(i<=PRIO_MAX) {
            //printf("i= %d\n", i);
            // Verifie si la file est vide : pas de process entre noPriorité et MAX et pas de process de MIN à noPriorité
            if((pasDeProcess == true) && (i == noPriorite)) {
                fileVide = true;
                //printf("fileVide ! break\n");
                break;
            }
            // S'il n'y a pas de process de priorité recherché, on passe à la priorité d'apres
            if ((msgrcv(msgid, &process, sizeof(processus) - 4, i, IPC_NOWAIT)) == -1) {
                if (i == PRIO_MAX) { // Si on atteint la priorité MAX et qu'il n'y a toujours pas de processus on recommence à MIN
                    i = PRIO_MIN-1;
                    pasDeProcess = true;
                }
            } else {
                // Dans le cas où un processus est trouvé
                fileVide = false;
                pasDeProcess = false;
                //sleep(1);
                printf("\n----------------------------  Quantum : %d      Cherche priorité %d  -----------------------------\n",
                       quantum, noPriorite);
                printf("[PROCESSUS SORTI DE LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n",
                       process.pid, process.priorite, process.tpsExec, process.dateSoumission);

                if (process.tpsExec - dureeQuantum > 0) {
                    process.tpsExec -= dureeQuantum;
                    process.priorite += 1;

                    if (msgsnd(msgid, &process, sizeof(processus) - 4, 0) == -1) {
                        perror("Erreur de lecture requete \n");
                        exit(1);
                    }
                    printf("[PROCESSUS MIS DANS LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n",
                           process.pid, process.priorite, process.tpsExec, process.dateSoumission);
                } else {
                    process.tpsExec = 0;
                    printf("[PROCESSUS TERMINE]\t\t\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n",
                           process.pid, process.priorite, process.tpsExec, process.dateSoumission);
                }
                break;
            }
            i++;
        }
        quantum++;
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
    printf("\n=================================   LANCEMENT DU GENERATEUR DE PROCESSUS   =================================\n");
    for (int i = 0; i < nbProcess; ++i) {
        processus process;
        process.pid = i+1;
        process.priorite = rand()%10 +1;
        process.tpsExec = rand()%5 +1;
        process.dateSoumission = rand()%11;
        printf("[PROCESSUS MIS DANS LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);

        // Envoi du processus dans la file de message
        if (msgsnd(msgid, &process, sizeof(processus) - 4,0) == -1) {
            perror("Erreur de lecture requete \n");
            exit(1);
        }
    }
    printf("============================================================================================================\n");
}
