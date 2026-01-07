#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "gameMemory.h"
#include <unistd.h>
#include <stdlib.h>

void symulacjaAtaku(struct GameMemory *gra, int graczAtakujacy) {
    int graczObraniajacy;
    if (graczAtakujacy == 0) {
        printf("Gracz 0 atakuje gracza 1!\n");
        graczObraniajacy = 1;
    } else {
        printf("Gracz 1 atakuje gracza 0!\n");
        graczObraniajacy = 0;
    }

    int silaAtaku = gra->gracze[graczAtakujacy].lpiechota * 1 + gra->gracze[graczAtakujacy].cpiechota * 1.5 + gra->gracze[graczAtakujacy].jazda * 3.5;
    int silaObrony = gra->gracze[graczObraniajacy].lpiechota * 1.2 + gra->gracze[graczObraniajacy].cpiechota * 3 + gra->gracze[graczObraniajacy].jazda * 1.2;
    printf("Siła ataku: %d vs Siła obrony: %d\n", silaAtaku, silaObrony);
    if (silaAtaku - silaObrony > 0) {
        printf("Wszystkie jednostki obrońcy zostały zniszczone!\n");
        gra->gracze[graczObraniajacy].lpiechota = 0;
        gra->gracze[graczObraniajacy].cpiechota = 0;
        gra->gracze[graczObraniajacy].jazda = 0;
    } else {
        printf("Atak został odparty!\n");
        printf("Straty broniącego:\n");
        int lekkaPiechotaStraty = gra->gracze[graczObraniajacy].lpiechota * (silaAtaku / silaObrony);
        int ciezkaPiechotaStraty = gra->gracze[graczObraniajacy].cpiechota * (silaAtaku / silaObrony);
        int jazdaStraty = gra->gracze[graczObraniajacy].jazda * (silaAtaku / silaObrony);
        printf("Lekka piechota: %d, Ciężka piechota: %d, Jazda: %d\n", lekkaPiechotaStraty, ciezkaPiechotaStraty, jazdaStraty);


        int silaAtaku = gra->gracze[graczObraniajacy].lpiechota * 1 + gra->gracze[graczObraniajacy].cpiechota * 1.5 + gra->gracze[graczObraniajacy].jazda * 3.5;
        int silaObrony = gra->gracze[graczAtakujacy].lpiechota * 1.2 + gra->gracze[graczAtakujacy].cpiechota * 3 + gra->gracze[graczAtakujacy].jazda * 1.2;

        printf("Straty atakującego:\n");
        int lekkaPiechotaStratyAtakujacy = gra->gracze[graczAtakujacy].lpiechota * (silaAtaku / silaObrony);
        int ciezkaPiechotaStratyAtakujacy = gra->gracze[graczAtakujacy].cpiechota * (silaAtaku / silaObrony);
        int jazdaStratyAtakujacy = gra->gracze[graczAtakujacy].jazda * (silaAtaku / silaObrony);
        printf("Lekka piechota: %d, Ciężka piechota: %d, Jazda: %d\n", lekkaPiechotaStratyAtakujacy, ciezkaPiechotaStratyAtakujacy, jazdaStratyAtakujacy);
        gra->gracze[graczObraniajacy].lpiechota -= lekkaPiechotaStraty;
        gra->gracze[graczObraniajacy].cpiechota -= ciezkaPiechotaStraty;
        gra->gracze[graczObraniajacy].jazda -= jazdaStraty;

        gra->gracze[graczAtakujacy].lpiechota -= lekkaPiechotaStratyAtakujacy;
        gra->gracze[graczAtakujacy].cpiechota -= ciezkaPiechotaStratyAtakujacy;
        gra->gracze[graczAtakujacy].jazda -= jazdaStratyAtakujacy;
    }
}

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

        //printf(".\n"); //test odpowiedzi programu
        fflush(stdout);

        for (int i = 0; i < 2; i++) {
            if(gra->gracze[i].komenda != CMD_BRAK) {
                
                printf("Gracz %d wydał komendę: %d\n", i, gra->gracze[i].komenda);

                // obsługa kupna robotnika  )
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
                // obsługa kupna lekkiej piechoty
                if(gra->gracze[i].komenda == CMD_KUP_LPIECHOTA) {
                    if (gra->gracze[i].surowce >= 100) {
                        gra->gracze[i].surowce -= 100;
                        gra->gracze[i].lpiechota += 1;
                        printf("-> Sukces. Gracz %d ma teraz %d lekkiej piechoty.\n", i, gra->gracze[i].lpiechota);
                    } else {
                        printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                    }
                }
                // obsługa kupna ciężkiej piechoty
                if(gra->gracze[i].komenda == CMD_KUP_CPIECHOTA) {
                    if (gra->gracze[i].surowce >= 250) {
                        gra->gracze[i].surowce -= 250;
                        gra->gracze[i].cpiechota += 1;
                        printf("-> Sukces. Gracz %d ma teraz %d ciężkiej piechoty.\n", i, gra->gracze[i].cpiechota);
                    } else {
                        printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                    }
                }
                // obsługa kupna jazdy
                if(gra->gracze[i].komenda == CMD_KUP_JAZDA) {
                    if (gra->gracze[i].surowce >= 550) {
                        gra->gracze[i].surowce -= 550;
                        gra->gracze[i].jazda += 1;
                        printf("-> Sukces. Gracz %d ma teraz %d jazdy.\n", i, gra->gracze[i].jazda);
                    } else {
                        printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                    }
                }
                // obsługa ataku
                if(gra->gracze[i].komenda == CMD_ATAK) {
                    symulacjaAtaku(gra, i);
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