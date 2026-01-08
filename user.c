#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "gameMemory.h"
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int shm_id = shmget(SHM_KEY, sizeof(struct GameMemory) , IPC_CREAT | 0640);

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

    while(1) {
        char command[50];
        fgets(command, sizeof(command), stdin);

        printf("\033[H\033[J"); // czyszczenie terminala

        if (strncmp(command, "wyjscie", 7) == 0) {
            break;
        } else if (strncmp(command, "help", 4) == 0){
            printf("Dostępne komendy:\n");
            printf("status - wyświetla aktualny stan zasobów i jednostek gracza\n"); //tymczasowe
            printf("kup <typ_jednostki> <ilość> - kupuje określoną ilość jednostek danego typu\n");
            printf("atak - atakuje drugiego gracza całą dostępną armią\n");
            printf("help - wyświetla pomoc\n");
            printf("wyjscie - kończy program\n");
        } else if (strncmp(command, "kup", 3) == 0) {
            if (gra ->gracze[id_gracza].komenda != CMD_BRAK) {
                printf("Nie można złożyć nowego zamówienia, poprzednie jest w trakcie realizacji.\n");
                continue;
            }

            if (strstr(command, "lpiechota") != NULL) {
                gra->gracze[id_gracza].komenda = CMD_KUP_LPIECHOTA;
                printf("Złożono zamówienie na lekką piechotę.\n");
            } else if (strstr(command, "cpiechota") != NULL) {
                gra->gracze[id_gracza].komenda = CMD_KUP_CPIECHOTA;
                printf("Złożono zamówienie na ciężką piechotę.\n");
            } else if (strstr(command, "jazda") != NULL) {
                gra->gracze[id_gracza].komenda = CMD_KUP_JAZDA;
                printf("Złożono zamówienie na jazdę.\n");
            } else if (strstr(command, "robotnik") != NULL) {
                gra->gracze[id_gracza].komenda = CMD_KUP_ROBOTNIKA;
                printf("Złożono zamówienie na robotnika.\n");
            } else {
                printf("Nieznany typ jednostki. Dostępne typy: lpiechota, cpiechota, jazda, robotnik.\n");
            }
        } else if (strncmp(command, "atak", 4) == 0) {
            gra->gracze[id_gracza].komenda = CMD_ATAK;
            printf("Złożono rozkaz ataku.\n");
        } else if (strncmp(command, "status", 6) == 0) {
            printf("Gracz %d - Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
               id_gracza,
               gra->gracze[id_gracza].surowce,
               gra->gracze[id_gracza].lpiechota,
               gra->gracze[id_gracza].cpiechota,
               gra->gracze[id_gracza].jazda,
               gra->gracze[id_gracza].robotnicy);
        } else {
            printf("Nieznana komenda. Wpisz 'help' aby uzyskać pomoc.\n");
        }
        

    }
}