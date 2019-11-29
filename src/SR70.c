#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>

#define CLE_FILE_PRINCIPALE  0x123
#define CLE_FILE_ATTENTE  0x456
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

int quantum = -1;
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

    printf("\n\n============================================================================================================\n");
    printf("=======================================  ***   DEBUT DE SR70   ***  ========================================\n");
    printf("============================================================================================================\n\n\n");

    //======= Données entrées par l'utilisateur ======
    printf("Entrez le nombre de processus à générer aléatoirement : ");
    scanf("%d", &nbProcess);

    dureeQuantum = 0;
    while(dureeQuantum < 1) {
        printf("Entrez la durée d'un quantum de temps (doit être supérieur ou égal à 1) : ");
        scanf("%d", &dureeQuantum);
    }
    //================================================


    //============= Ouverture du fichier TableCPU.csv ===============
    FILE* fichTabCPU = fopen("TableCPU.csv", "r");
    if (fichTabCPU==NULL) {
        printf("Ouverture fichier impossible !");
        exit(1);
    }//==============================================================

    //======== Lecture du fichier TableCPU.csv =========
    while (fgets(chaine, 5, fichTabCPU) != NULL) {
        tempTabCPU[tailleTabCPU] = atoi(chaine);
        tailleTabCPU++;
    }
    fclose(fichTabCPU);
    //==================================================


    //============== Ouverture du fichier JeuDeTest.csv ================
    FILE* fichJeuDeTest = fopen("JeuDeTest.csv", "r");
    if (fichJeuDeTest==NULL) {
        printf("Ouverture fichier impossible !");
        exit(1);
    }//=================================================================

    //============= Lecture du fichier JeuDeTest.csv ======================
    int j=0;
    while (fscanf(fichJeuDeTest, "%d;%ld;%d;%d",&processJeuDeTest[j].pid, &processJeuDeTest[j].priorite, &processJeuDeTest[j].tpsExec, &processJeuDeTest[j].dateSoumission) != EOF) {
        j++;
    }
    fclose(fichJeuDeTest);
    //=====================================================================


    //====== Remplissage table d'allocation CPU ======
    tabCPU = malloc(tailleTabCPU * sizeof(int));

    for(int i=0; i<tailleTabCPU; i++) {
        tabCPU[i] = tempTabCPU[i];
    }//===============================================


    //=====================   Création des files de messages   ===========================================
    // Supprime les files si elles existent encore
    if ((msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_CREAT)) != -1) {
        msgctl(msgid_Principale, IPC_RMID, NULL);
    }
    if((msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_CREAT)) != -1) {
        msgctl(msgid_Attente, IPC_RMID, NULL);
    }

    // Crée les files
    if ((msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file principale\n");
        exit(1);
    }
    if ((msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_CREAT | IPC_EXCL)) == -1) {
        perror("Erreur de creation de la file d'attente\n");
        exit(1);
    }//================================================================================================


    printf("\n======================================   LECTURE DU JEU DE TEST CSV   ======================================\n");
    for(int i=0; i<NB_PROCESS_TEST; i++) {
        printf("[PROCESSUS GENERE]\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", processJeuDeTest[i].pid, processJeuDeTest[i].priorite, processJeuDeTest[i].tpsExec, processJeuDeTest[i].dateSoumission);
        EnvoiProcessus(processJeuDeTest[i]);
    }
    printf("============================================================================================================\n");


    //======= Génerateur de processus et superviseur =======
    ProcessusGenerateur(nbProcess);
    Superviseur(tabCPU, tailleTabCPU, dureeQuantum);
    //======================================================


    printf("\n\n============================================================================================================\n");
    printf("========================================  ***   FIN DE SR70   ***  =========================================\n");
    printf("============================================================================================================\n\n\n");

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
    quantum = 0;
    // Tant que la file n'est pas vide et qu'il reste des processus
    while(!filePrincipaleVide || !fileAttenteVide) {
        processus process;
        int noPrioriteBase = tabCPU[quantum % tailleTabCPU]; // priorité cherchée

        printf("\n----------------------------------  Quantum : %d \tCherche priorité %d  -----------------------------------\n", quantum, noPrioriteBase);

        VerifProcessusAttente();

        // S'il n'y a pas de process de priorité recherché, on passe à la priorité +1
        for(int i=0; i<PRIO_MAX; i++) {
            int noPriorite;

            if( (noPrioriteBase+i)>(PRIO_MAX) ) {
                noPriorite = (i+noPrioriteBase)%(PRIO_MAX+1) +1;
            } else {noPriorite = (i+noPrioriteBase)%(PRIO_MAX+1);}

            // Un processus est trouvé
            if ((msgrcv(msgid_Principale, &process, sizeof(processus) - sizeof(long), noPriorite, IPC_NOWAIT)) != -1) {
                printf("[PROCESSUS SORTI ET EXECUTE]\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
                TraitementProcessus(process, dureeQuantum);
                filePrincipaleVide = false;
                break;
            }
            if (i == PRIO_MAX-1) {filePrincipaleVide = true;}
        }
        quantum++;
    }
    printf("\nPlus de processus dans les files de messages.\n");
    printf("============================================================================================================\n");
}



// Générateur de processus aléatoire
void ProcessusGenerateur(int nbProcess) {
    printf("\n\n=================================   LANCEMENT DU GENERATEUR DE PROCESSUS   =================================\n");
    for (int i = 0; i < nbProcess; ++i) {
        processus process;
        process.pid = i+NB_PROCESS_TEST+1;
        process.priorite = rand()%10 +1;
        process.tpsExec = rand()%7 +1;
        process.dateSoumission = rand()%14;

        printf("[PROCESSUS GENERE]\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
        EnvoiProcessus(process);
    }
    printf("============================================================================================================\n");
}


void EnvoiProcessus(processus process){
    int msgid_Principale = msgget(CLE_FILE_PRINCIPALE, 0750 | IPC_EXCL);
    int msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_EXCL);

    if ((process.dateSoumission <= quantum) && (quantum >= 0)) {
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
    }
}


void VerifProcessusAttente() {
    int msgid_Attente = msgget(CLE_FILE_ATTENTE, 0750 | IPC_EXCL);
    processus process;
    int pidVal =- 1;
    bool premierAEteLu = false;

    // Tant qu'il y a des processus dans la file attente
    int i =0;
    while((msgrcv(msgid_Attente, &process, sizeof(processus) - sizeof(long), 0, IPC_NOWAIT)) != -1) {

        EnvoiProcessus(process);

        if((premierAEteLu == false) && (process.dateSoumission > quantum)) {
            pidVal = process.pid;
            premierAEteLu = true;
        } else if(pidVal == process.pid) {
        // Sort du while si on a fait un tour de la file attente
            break;
        }
        i++;
    }

    if(i==0) {
        fileAttenteVide = true;
    }
}



void TraitementProcessus(processus process, int dureeQuantum) {
    if (process.tpsExec - dureeQuantum > 0) {
        process.tpsExec -= dureeQuantum;
        if (process.priorite < PRIO_MAX) { process.priorite += 1; }

        EnvoiProcessus(process);

    } else {
        process.tpsExec = 0;
        printf("[PROCESSUS TERMINE]\t\tPID : %d \t priorite : %ld \t temps d'execution : %d \t date de soumission : %d\n", process.pid, process.priorite, process.tpsExec, process.dateSoumission);
    }
}