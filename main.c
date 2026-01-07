#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "gameMemory.h"
#include <unistd.h>
#include <stdlib.h>

int main() {
    int shm_id = shmget(SHM_KEY, sizeof(struct GameMemory) , IPC_CREAT | 0640);
    //int surowce = 300;
    //int lpiechota = 0; //100 1A 1.2O 2s
    //int cpiechota = 0; //250 1.5A 3O 3s
    //int jazda = 0; //550 3.5A 1.2O 5s
    //int robotnicy = 0; //150 0A 0O 2s

    struct GameMemory *gra = (struct GameMemory*) shmat(shm_id, NULL, 0);
    //Inicjalizacja 
    gra->gracze[0].surowce = 300;
    gra->gracze[0].lpiechota = 0;
    gra->gracze[0].cpiechota = 0;
    gra->gracze[0].jazda = 0;
    gra->gracze[0].robotnicy = 0;

    gra->gracze[1].surowce = 300;
    gra->gracze[1].lpiechota = 0;
    gra->gracze[1].cpiechota = 0;
    gra->gracze[1].jazda = 0;
    gra->gracze[1].robotnicy = 0;

    gra->gra_aktywna = 0;

    printf("Inicjalizacja pamięci zakończona.\n");


    printf("Gracz 0 - Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
           gra->gracze[0].surowce,
           gra->gracze[0].lpiechota,
           gra->gracze[0].cpiechota,
           gra->gracze[0].jazda,
           gra->gracze[0].robotnicy);

    printf("Gracz 1 - Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
           gra->gracze[1].surowce,
           gra->gracze[1].lpiechota,
           gra->gracze[1].cpiechota,
           gra->gracze[1].jazda,
           gra->gracze[1].robotnicy);

    int OstCzas = 0;
    while(1) {

        // dodawanie surowców co turę
        if (OstCzas == 100) {
            gra->gracze[0].surowce += 50 + (gra->gracze[0].robotnicy * 5);
            gra->gracze[1].surowce += 50 + (gra->gracze[1].robotnicy * 5);
            OstCzas = 0;
        }

        printf(".\n"); //test odpowiedzi programu
        fflush(stdout);

        for (int i = 0; i < 2; i++) {
            if(gra->gracze[i].komenda != CMD_BRAK) {
                
                printf("Gracz %d wydał komendę: %d\n", i, gra->gracze[i].komenda);

                // obsługa kupna robotnika
                if(gra->gracze[i].komenda == CMD_KUP_ROBOTNIKA) {
                    // dodaj zabezpieczenie: czy go stać?
                    if (gra->gracze[i].surowce >= 150) {
                        gra->gracze[i].surowce -= 150;
                        gra->gracze[i].robotnicy += 1;
                        printf("-> Sukces. Gracz %d ma teraz %d robotników.\n", i, gra->gracze[i].robotnicy);
                    } else {
                        printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                    }
                }

                // czyścimy komendę po wykonaniu
                gra->gracze[i].komenda = CMD_BRAK;
            }
        }
        OstCzas++;
        usleep(10000); // opóźnienie 0.01 sekundy
    }

    //zamykanie pamięci współdzielonej
    shmdt(gra);
    return 0;
}