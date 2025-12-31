#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "gameMemory.h"

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
    }