#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>

#define CLE_FILE_PRINCIPALE  0x123
#define CLE_FILE_ATTENTE  0x456
#define PRIO_MIN  1
#define PRIO_MAX  10
#define NB_PROCESS_TEST  5

typedef struct {
    long priorite;
    int pid;
    int dateSoumission;
    int tpsExec;
} processus;


void ProcessusGenerateur(int);
void Superviseur(int*, int, int);
void EnvoiProcessus(processus);
void VerifProcessusAttente();
void TraitementProcessus(processus, int);

int quantum = 0;
bool fileAttenteVide = false;

int main() {
    int msgid_Principale; //id de la file de messages
    int msgid_Attente; //id de la file de messages
    int nbProcess;
    int dureeQuantum;
    int* tabCPU; // tableau contenant la table d'allocation CPU
    int tempTabCPU[200]; // tableau temporaire pour remplir la table CPU
    int tailleTabCPU = 0; // taille de la table CPU
    char chaine[5]; // variable tampon pour récupérer les valeurs du fichier csv
    processus processJeuDeTest[NB_PROCESS_TEST];


    //======= Données entrées par l'utilisateur ======
    printf("Entrez le nombre de processus : ");
    scanf("%d", &nbProcess);

    printf("Entrez la durée d'un quantum : ");
    scanf("%d", &dureeQuantum);
    //================================================


    //============= Ouverture du fichier TableCPU.csv ===============
    FILE* fichTabCPU = fopen("../TableCPU.csv", "r");
    if (fichTabCPU==NULL) {
        printf("Ouverture fichier impossible !");
        exit(1);
    }//==============================================================

    //============ Lecture du fichier TableCPU.csv =============
    while (fgets(chaine, 5, fichTabCPU) != NULL) {
        tempTabCPU[tailleTabCPU] = atoi(chaine);
        tailleTabCPU++;
    }
    fclose(fichTabCPU);
    //==========================================================


    //============== Ouverture du fichier JeuDeTest.csv ================
    FILE* fichJeuDeTest = fopen("../JeuDeTest.csv", "r");
    if (fichJeuDeTest==NULL) {
        printf("Ouverture fichier impossible !");
        exit(1);
    }//=================================================================

    //============= Lecture du fichier JeuDeTest.csv ======================
    int j=0;
    while (fscanf(fichJeuDeTest, "%d;%ld;%d;%d",&processJeuDeTest[j].pid, &processJeuDeTest[j].priorite, &processJeuDeTest[j].tpsExec, &processJeuDeTest[j].dateSoumission) != EOF) {
        //printf("");
        j++;
    }
    fclose(fichJeuDeTest);
    //===========================================================


    //====== Remplissage table d'allocation CPU ======
    tabCPU = malloc(tailleTabCPU * sizeof(int));

    for(int i=0; i<tailleTabCPU; i++) {
        tabCPU[i] = tempTabCPU[i];
    }//===============================================


    //================ Création des files de messages======================
    // Supprime la file si elle existe encore
    if ((msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_CREAT)) != -1) {
        msgctl(msgid_Principale, IPC_RMID, NULL);
    }
    if((msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_CREAT)) != -1) {
        msgctl(msgid_Attente, IPC_RMID, NULL);
    }

    // Crée la file
    if ((msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file principale\n");
        exit(1);
    }
    if ((msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file d'attente\n");
        exit(1);
    }
    //=====================================================================

    printf("\n======================================   LECTURE DU JEU DE TEST CSV   ======================================\n");
    for(int i=0; i<NB_PROCESS_TEST; i++) {
        printf("[PROCESSUS GENERE]\t\t\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", processJeuDeTest[i].pid, processJeuDeTest[i].priorite, processJeuDeTest[i].tpsExec, processJeuDeTest[i].dateSoumission);
        EnvoiProcessus(processJeuDeTest[i]);
    }
    printf("============================================================================================================\n");


    //======= Génerateur de processus et superviseur =======
    ProcessusGenerateur(nbProcess);
    Superviseur(tabCPU, tailleTabCPU, dureeQuantum);
    //======================================================


    //=== Suppr file de msg et libération mémoire ===
    msgctl(msgid_Principale, IPC_RMID, NULL);
    msgctl(msgid_Attente, IPC_RMID, NULL);
    free(tabCPU);
    //===============================================

    return 0;
}



void Superviseur(int* tabCPU, int tailleTabCPU, int dureeQuantum) {
    int msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_EXCL);
    bool filePrincipaleVide = false;

    printf("\n\n=====================================   LANCEMENT DU SUPERVISEUR   =========================================\n");

    // Tant que la file n'est pas vide et qu'il reste des processus
    while(!filePrincipaleVide || !fileAttenteVide) {
        processus process;
        int noPrioriteBase = tabCPU[quantum % tailleTabCPU]; // priorité cherchée

        printf("\n----------------------------------  Quantum : %d \tCherche priorité %d  -----------------------------------\n", quantum, noPrioriteBase);

        VerifProcessusAttente();

        // S'il n'y a pas de process de priorité recherché, on passe à la priorité d'apres
        for(int i=0; i<PRIO_MAX; i++) {
            int noPriorite;

            if( (noPrioriteBase+i)>(PRIO_MAX) ) {
                noPriorite = (i+noPrioriteBase)%(PRIO_MAX+1) +1;
            } else {noPriorite = (i+noPrioriteBase)%(PRIO_MAX+1);}

//            printf("no prio : %d\n", noPriorite);

            // Un processus est trouvé
            if ((msgrcv(msgid_Principale, &process, sizeof(processus) - sizeof(long), noPriorite, IPC_NOWAIT)) != -1) {
//                printf("[PROCESSUS SORTI DE LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
                printf("[PROCESSUS SORTI ET EXECUTE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
                TraitementProcessus(process, dureeQuantum);
                filePrincipaleVide = false;
                break;
            }
            if (i == PRIO_MAX-1) {filePrincipaleVide = true;}
        }
        quantum++;
//        sleep(1);
    }
    printf("============================================================================================================\n");
}



// Générateur de processus aléatoire
void ProcessusGenerateur(int nbProcess) {
    printf("\n\n=================================   LANCEMENT DU GENERATEUR DE PROCESSUS   =================================\n");
    for (int i = 0; i < nbProcess; ++i) {
        processus process;
        process.pid = i+6;
        process.priorite = rand()%10 +1;
        process.tpsExec = rand()%5 +1;
        process.dateSoumission = rand()%11;

        printf("[PROCESSUS GENERE]\t\t\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
        EnvoiProcessus(process);
//        sleep(1);
        // Envoi du processus dans la file de message
//        if (msgsnd(msgid, &process, sizeof(processus) - 4,0) == -1) {
//            perror("Erreur de lecture requete \n");
//            exit(1);
//        }
//        printf("[PROCESSUS MIS DANS LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
    }
    printf("============================================================================================================\n");
}


void EnvoiProcessus(processus process){
    int msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_EXCL);
    int msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_EXCL);

    //
//    printf("\ndatesoumission : %d = quantum : %d\n",process.dateSoumission, quantum);

    if (process.dateSoumission <= quantum) {
        if (msgsnd(msgid_Principale, &process, sizeof(processus) - sizeof(long), 0) == -1) {
            perror("Erreur de lecture requete msgsnd file principale\n");
            exit(1);
        }
        printf("[PROCESSUS MIS DANS LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n",process.pid, process.priorite, process.tpsExec, process.dateSoumission);
    } else {
        if (msgsnd(msgid_Attente, &process, sizeof(processus) - sizeof(long), 0) == -1) {
            perror("Erreur de lecture requete msgsnd file attente\n");
            exit(1);
        }
//        printf("[PROCESSUS EN ATTENTE]\t\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n",process.pid, process.priorite, process.tpsExec, process.dateSoumission);
    }
}


void VerifProcessusAttente() {
//    int msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_EXCL);
    int msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_EXCL);
    processus process;
    int pidVal =- 1;
    bool premierAEteLu = false;

    /*if((msgrcv(msgid_Attente, &process, sizeof(processus) - sizeof(long), 0, IPC_NOWAIT)) != -1) {
        pidVal = process.pid;
        if (msgsnd(msgid_Attente, &process, sizeof(processus) - sizeof(long), 0) == -1) {
            perror("Erreur de lecture requete msgsnd file principale\n");
            exit(1);
        }
    }*/

    // Tant qu'il y a des processus dans la file attente
    int i =0;
    while((msgrcv(msgid_Attente, &process, sizeof(processus) - sizeof(long), 0, IPC_NOWAIT)) != -1) {
//        printf("[WHILE]\t\t\t\t\t\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n",process.pid, process.priorite, process.tpsExec, process.dateSoumission);

        EnvoiProcessus(process);

        if((premierAEteLu == false) && (process.dateSoumission > quantum)) {
            pidVal = process.pid;
            premierAEteLu = true;
        } else if(pidVal == process.pid) {
        // Sort du while si on a fait un tour de la file attente
//            printf("break while\n");
            break;
        }

//        sleep(1);
        i++;
    }

    if(i==0) {
        fileAttenteVide = true;
    }
}



void TraitementProcessus(processus process, int dureeQuantum) {
//    printf("[PROCESSUS SORTI DE LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);

    if (process.tpsExec - dureeQuantum > 0) {
        process.tpsExec -= dureeQuantum;
        if (process.priorite < PRIO_MAX) { process.priorite += 1; }

        EnvoiProcessus(process);
//        if (msgsnd(msgid_Principale, &process, sizeof(processus) - 4, 0) == -1) {
//            perror("Erreur de lecture requete \n");
//            exit(1);
//        }
//        printf("[PROCESSUS MIS DANS LA FILE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
    } else {
        process.tpsExec = 0;
        printf("[PROCESSUS TERMINE]\t\t\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
    }
}