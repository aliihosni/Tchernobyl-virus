#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

/** Definition de la structure capteur */
typedef struct Capteur {
    pid_t pid;
    int total;
    int isKilled;
} Capteur ;

/** Declaration des methodes */
void perehandler(int);
void ecrire(char*);
void centre(int, int, int, Capteur*, FILE*);// int:pid du fils, int:son niveau de danger, int:nb total des fils, Capteur*:tab des capteurs, FILE*: journal du pere
int nbFilsKilled(int, Capteur*);// int: longeur du tableau, Capteur*: tableau des capteurs
int totalDepassement(int, Capteur*);// Calculer le total des passement


int main(int nb_arg, char* tab_arg[]){

    Capteur *tabCapteur;
    Capteur capt;
    int n,i, status=0, scannedpid, scannedval, pidfils;
    char *fd = "niveaux_danger.txt", *journald, *nbfils;
    FILE *fp, *journal;
    time_t t;
    char line[255];


      journald=tab_arg[1];
      nbfils=tab_arg[2];

      //conversion de nbfils de chaine de caracteres en integer
      n = atoi(nbfils);

    /** Allocation dynamique du tableau des capteurs */
    tabCapteur = (Capteur*) malloc (n*sizeof(Capteur));


    /** Creation des fils */
    for(i=0; i<n; i++){
        if(!(pidfils=fork())){

            ecrire(fd);
        }
        else{
            tabCapteur[i].pid = pidfils;
            tabCapteur[i].total = 0;
            tabCapteur[i].isKilled = 0;
        }
    }

    /** Corps */
    //pour attendre que le premier fils ecrire dans le fichier
    signal(SIGUSR1, perehandler);
    pause();

    fp = fopen(fd, "r");

    while(fscanf(fp,"%d %d", &scannedpid, &scannedval)>0){

        journal = fopen(journald, "a");
        centre(scannedpid, scannedval, n, tabCapteur, journal);
        fclose(journal);

        if(nbFilsKilled(n, tabCapteur)==n) break;


        pause();
    }
    fclose(fp);

    journal = fopen(journald, "a");
    fprintf(journal, "Le total des depassement est %d\nLa moyenne des depassememnt est %.2f", totalDepassement(n,tabCapteur), (float)((float)totalDepassement(n,tabCapteur)/(float)n));
    fclose(journal);
    /** end corps */

    return 0;
}

/** Implementation des methodes */

void perehandler(int sig){
    signal(sig, perehandler);
}

void centre(int pid, int niv, int n, Capteur* tabc, FILE* j){
    int i;

    for(i=0; i<n; i++){
        if (tabc[i].pid == pid){
            tabc[i].total+=niv;
            fprintf(j, "fils: %d niveau: %d total: %d\n", pid, niv, tabc[i].total);
            if(tabc[i].total>=100 && !tabc[i].isKilled){
                kill(pid, SIGKILL);
                tabc[i].isKilled=1;
                fprintf(j,"Un signal SIGKILL est envoyey au fils %d\n", pid);
            }
            break;
        }

    }
}

int nbFilsKilled(int n, Capteur* tabc){

    int nbfk=0; //nombre de fils deja killed
    int i;

    for(i=0; i<n; i++){
        if(tabc[i].isKilled==1) nbfk++;
    }

    return nbfk;
}

int totalDepassement(int n, Capteur* tabc){

    int tot=0; //nombre total des depassememnt
    int i;

    for(i=0; i<n; i++){
        if(tabc[i].total>100) tot+=(tabc[i].total - 100);
    }

    return tot;
}

void ecrire(char* fd){

    time_t t,seconds;
    FILE* fp;



    srand((unsigned)time(&t)+getpid());

    while(1){

        sleep(1);
        fp = fopen(fd, "a+");
        seconds = time(NULL);
        fprintf(fp,"%d %d\n", getpid(), rand()%9+1);
        fclose(fp);
        kill(getppid(), SIGUSR1);
    }
}
