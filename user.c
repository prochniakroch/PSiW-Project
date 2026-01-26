#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include "gameMemory.h"
#include <string.h>
#include <unistd.h>

// --- SEMAFORY ---
static struct sembuf buf;

void podnies(int sem_id, int semnum) {
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;

    if (semop(sem_id, &buf, 1) == -1) {
        perror("błąd podnoszenia semafora");
    }
}

void opusc(int sem_id, int semnum) {
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    
    if (semop(sem_id, &buf, 1) == -1) {
        perror("błąd opuszczania semafora");
    }
}


// --- PROGRAM GŁÓWNY ---
int main(int argc, char *argv[]) {
    int shm_id = shmget(SHM_KEY, sizeof(struct GameMemory) , IPC_CREAT | 0640);
    int sem_id = semget(SEM_KEY, 1, 0);

    struct GameMemory *gra = (struct GameMemory*) shmat(shm_id, NULL, 0);

    if (argc != 2) {
        printf("Użycie: %s <id_gracza>\n", argv[0]);
        return 1;
    }
    if (argv[1][0] < '0' || argv[1][0] > '1') {
        printf("Nieprawidłowy id_gracza. Dozwolone wartości: 0 lub 1.\n");
        return 1;
    }
    int id_gracza = argv[1][0] - '0';

    printf("Gracz %d - Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
       id_gracza,
       gra->gracze[id_gracza].surowce,
       gra->gracze[id_gracza].lpiechota,
       gra->gracze[id_gracza].cpiechota,
       gra->gracze[id_gracza].jazda,
       gra->gracze[id_gracza].robotnicy);

    printf("Wpisz komendę (format: <typ_jednostki> <ilość>) lub 'wyjscie' aby zakończyć:\n");   

    pid_t pid = fork();
    

    if (pid == 0) {
        // Proces potomny - nasłuchiwanie komunikatów od serwera
        while(1) {
            if (gra->gracze[id_gracza].czyNowyKomunikat == 1) {
                opusc(sem_id, 0);
                printf("%s\n", gra->gracze[id_gracza].komunikat);
                gra->gracze[id_gracza].czyNowyKomunikat = 0;
                podnies(sem_id, 0);
            }
            if (gra->gracze[id_gracza].zmianaStanuZasobow == 1) {
                opusc(sem_id, 0);
                printf("Gracz %d - Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
                id_gracza,
                gra->gracze[id_gracza].surowce,
                gra->gracze[id_gracza].lpiechota,
                gra->gracze[id_gracza].cpiechota,
                gra->gracze[id_gracza].jazda,
                gra->gracze[id_gracza].robotnicy);
                gra->gracze[id_gracza].zmianaStanuZasobow = 0;
                podnies(sem_id, 0);
            }
            usleep(50000); // opóźnienie 0.1 sekundy
        }
    } else {
        // Proces macierzysty - wysyłanie komend do serwera
        while(1) {
            char command[50];
            char akcja[20];
            char coKupic[20];
            int ileKupic;

            // Pobierz komendę od użytkownika
            fgets(command, sizeof(command), stdin);
            int liczbaElementow = sscanf(command, "%s %s %d", akcja, coKupic, &ileKupic);

            // Przetwarzanie komendy -> wysyłanie do serwera
            if (liczbaElementow == 1) { 
                // 
                if (strncmp(akcja, "wyjscie", 7) == 0) {
                    kill(pid, SIGKILL); 
                    shmdt(gra);
                    break;
                } else if (strncmp(akcja, "help", 4) == 0){
                    printf("Dostępne komendy:\n");
                    printf("status - wyświetla aktualny stan zasobów i jednostek gracza\n"); //tymczasowe
                    printf("kup <typ_jednostki> <ilość> - kupuje określoną ilość jednostek danego typu\n");
                    printf("atak - atakuje drugiego gracza całą dostępną armią\n");
                    printf("help - wyświetla pomoc\n");
                    printf("wyjscie - kończy program\n");
                } else if (strncmp(akcja, "atak", 4) == 0) {
                    opusc(sem_id, 0);
                    gra->gracze[id_gracza].komenda = CMD_ATAK;
                    printf("Złożono rozkaz ataku.\n");
                    podnies(sem_id, 0);
                } else if (strncmp(akcja, "status", 6) == 0) {
                    opusc(sem_id, 0);
                    printf("Gracz %d - Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
                    id_gracza,
                    gra->gracze[id_gracza].surowce,
                    gra->gracze[id_gracza].lpiechota,
                    gra->gracze[id_gracza].cpiechota,
                    gra->gracze[id_gracza].jazda,
                    gra->gracze[id_gracza].robotnicy);
                    podnies(sem_id, 0);
                } else {
                    printf("Nieznana komenda. Wpisz 'help' aby uzyskać pomoc.\n");
                }
            } else if (liczbaElementow == 3) {
                if (strncmp(akcja, "kup", 3) == 0) {
                    if (ileKupic <= 0) {
                        printf("Nieprawidłowa ilość jednostek do kupienia. Musi być większa od 0.\n");
                        continue;
                    }
                    opusc(sem_id, 0);
                    if (strncmp(coKupic, "lpiechota", 14) == 0) {
                        gra->gracze[id_gracza].komenda = CMD_KUP_LPIECHOTA;
                        gra->gracze[id_gracza].komendaIlosc = ileKupic;
                        printf("Złożono rozkaz zakupu %d lekkiej piechoty.\n", ileKupic);
                    } else if (strncmp(coKupic, "cpiechota", 16) == 0) {
                        gra->gracze[id_gracza].komenda = CMD_KUP_CPIECHOTA;
                        gra->gracze[id_gracza].komendaIlosc = ileKupic;
                        printf("Złożono rozkaz zakupu %d ciężkiej piechoty.\n", ileKupic);
                    } else if (strncmp(coKupic, "jazda", 5) == 0) {
                        gra->gracze[id_gracza].komenda = CMD_KUP_JAZDA;
                        gra->gracze[id_gracza].komendaIlosc = ileKupic;
                        printf("Złożono rozkaz zakupu %d jazdy.\n", ileKupic);
                    } else if (strncmp(coKupic, "robotnik", 8) == 0) {
                        gra->gracze[id_gracza].komenda = CMD_KUP_ROBOTNIKA;
                        gra->gracze[id_gracza].komendaIlosc = ileKupic;
                        printf("Złożono rozkaz zakupu %d robotników.\n", ileKupic);
                    } else {
                        printf("Nieznany typ jednostki. Dostępne typy: lpiechota, cpiechota, jazda, robotnik.\n");
                    }
                    podnies(sem_id, 0);
                } else {
                    printf("Nieznana komenda. Wpisz 'help' aby uzyskać pomoc.\n");
                }
            } else {
                printf("Nieznana komenda. Wpisz 'help' aby uzyskać pomoc.\n");
            }
        }
    }
    kill(pid, SIGKILL); 
    shmdt(gra);
}