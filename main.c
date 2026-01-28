#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "gameMemory.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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


// --- ZMIENNE GLOBALNE ---
int sekunda = 0;
int czasAtaku = -1;
int ktoAtakuje = -1;
int czasProdukcji[2] = {-1, -1};


// --- FUNKCJE ---
void sprawdzCzyWygrana(struct GameMemory *gra, int gracz){
    if (gra->gracze[gracz].iloscWygranychAtakow >= 5) {
        printf("Gracz %d wygrał grę!\n", gracz);
        if (gracz == 0) {
            strcpy(gra->gracze[0].komunikat, "[SERWER] Wygrałeś grę!\n");
            strcpy(gra->gracze[1].komunikat, "[SERWER] Przegrałeś grę!\n");
        } else if (gracz == 1) {
            strcpy(gra->gracze[1].komunikat, "[SERWER] Wygrałeś grę!\n");
            strcpy(gra->gracze[0].komunikat, "[SERWER] Przegrałeś grę!\n");
        }
        gra->gracze[0].czyNowyKomunikat = 1;
        gra->gracze[1].czyNowyKomunikat = 1;
        gra->gra_aktywna = 0;
    }
}

int sprawdzCzyMozeAtakowac(struct GameMemory *gra, int graczAtakujacy) {
    if (gra->gracze[graczAtakujacy].lpiechota * 1 + gra->gracze[graczAtakujacy].cpiechota * 1.5 + gra->gracze[graczAtakujacy].jazda * 3.5 == 0) {
        printf("Atakujący nie ma jednostek do ataku!\n");
        strcpy(gra->gracze[graczAtakujacy].komunikat, "[SERWER] Nie masz jednostek do ataku!\n");
        gra->gracze[graczAtakujacy].czyNowyKomunikat = 1;
        return 0;
    } else {
        return 1;
    }
}

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

    if (silaObrony == 0) {
        printf("Obrońca nie ma jednostek do obrony! Wszystkie jednostki atakującego przechodzą bez strat.\n");
        strcpy(gra->gracze[graczAtakujacy].komunikat, "[SERWER] Wszystkie twoje jednostki przeszły bez strat!\n");
        gra->gracze[graczAtakujacy].czyNowyKomunikat = 1;
        gra->gracze[graczAtakujacy].iloscWygranychAtakow += 1;
        return;
    }

    if (silaAtaku - silaObrony > 0) {
        printf("Wszystkie jednostki obrońcy zostały zniszczone!\n");
        gra->gracze[graczObraniajacy].lpiechota = 0;
        gra->gracze[graczObraniajacy].cpiechota = 0;
        gra->gracze[graczObraniajacy].jazda = 0;
        gra->gracze[graczAtakujacy].iloscWygranychAtakow += 1;
    } else {
        //OBLICZANIE STRAT PODCZAS ATAKU
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

        ktoAtakuje = -1;
        czasAtaku = -1;
    }
}

void doAtaku(struct GameMemory *gra, int graczAtakujacy, int liczbaLP, int liczbaCP, int liczbaJazdy) {
    gra->gracze[graczAtakujacy].lpiechota -= liczbaLP;
    gra->gracze[graczAtakujacy].cpiechota -= liczbaCP;
    gra->gracze[graczAtakujacy].jazda -= liczbaJazdy;
    gra->gracze[graczAtakujacy].podczasAtaku.lpiechota = liczbaLP;
    gra->gracze[graczAtakujacy].podczasAtaku.cpiechota = liczbaCP;
    gra->gracze[graczAtakujacy].podczasAtaku.jazda = liczbaJazdy;
}

void poAtaku(struct GameMemory *gra, int graczAtakujacy, int liczbaLP, int liczbaCP, int liczbaJazdy) {
    gra->gracze[graczAtakujacy].podczasAtaku.lpiechota = 0;
    gra->gracze[graczAtakujacy].podczasAtaku.cpiechota = 0;
    gra->gracze[graczAtakujacy].podczasAtaku.jazda = 0;
    gra->gracze[graczAtakujacy].lpiechota += liczbaLP;
    gra->gracze[graczAtakujacy].cpiechota += liczbaCP;
    gra->gracze[graczAtakujacy].jazda += liczbaJazdy;
}

// --- FUNCKJE ZWIĄZANE Z PRODUKCJĄ ---
void inicjujProdukcje(struct GameMemory *gra) {
    for (int i = 0; i < MAX_PRODUKCJA; i++) {
        gra->gracze[0].produkcja[i].czyWolne = 0;
        gra->gracze[1].produkcja[i].czyWolne = 0;
        gra->gracze[0].produkcja[i].czas_pozostaly = 0;
        gra->gracze[1].produkcja[i].czas_pozostaly = 0;
    }
}

void dodajProdukcja(struct GameMemory *gra, int gracz, int typJednostki, int ilosc, int czasProdukcji) {
    int szukajWolnego = -1;
    for (int i = 0; i < MAX_PRODUKCJA; i++) {
        if (gra->gracze[gracz].produkcja[i].czyWolne == 0) {
            szukajWolnego = i;
            break;
        }
    }
    
    if (szukajWolnego == -1) {
        printf("Brak wolnego miejsca na produkcję dla gracza %d!\n", gracz);
        return;
    }
    gra->gracze[gracz].produkcja[szukajWolnego].czyWolne = 1;
    gra->gracze[gracz].produkcja[szukajWolnego].typ_jednostki = typJednostki;
    gra->gracze[gracz].produkcja[szukajWolnego].ilosc = ilosc;
    gra->gracze[gracz].produkcja[szukajWolnego].czas_pozostaly = czasProdukcji;

    printf("Dodano produkcję jednostki typu %d (ilość: %d, czas: %d) dla gracza %d na pozycję %d.\n", typJednostki, ilosc, czasProdukcji, gracz, szukajWolnego);
}

void aktualizujProdukcja(struct GameMemory *gra, int gracz, int sem_id) {
    for (int i = 0; i < MAX_PRODUKCJA; i++) {
        if (gra->gracze[gracz].produkcja[i].czyWolne == 1) {
            opusc(sem_id, 0);
            switch (gra->gracze[gracz].produkcja[i].typ_jednostki) {
                case 1: //lekka piechota
                    gra->gracze[gracz].lpiechota += 1;
                    break;
                case 2: //ciężka piechota
                    gra->gracze[gracz].cpiechota += 1;
                    break;
                case 3: //jazda
                    gra->gracze[gracz].jazda += 1;
                    break;
                case 4: //robotnik
                    gra->gracze[gracz].robotnicy += 1;
                    break;
            }
            podnies(sem_id, 0);
            if (gra->gracze[gracz].produkcja[i].ilosc > 1) {
                gra->gracze[gracz].produkcja[i].ilosc -= 1;
            } else {
                gra->gracze[gracz].produkcja[i].czyWolne = 0;
                gra->gracze[gracz].produkcja[i].czas_pozostaly = 0;
                gra->gracze[gracz].produkcja[i].ilosc = 0;
                gra->gracze[gracz].produkcja[i].typ_jednostki = 0;
            }
        }
    }
}

void przesunProdukcja(struct GameMemory *gra, int gracz) {
    for (int i = 0; i < MAX_PRODUKCJA; i++) {
        gra->gracze[gracz].produkcja[i] = gra->gracze[gracz].produkcja[i + 1];
    }
}

// --- PROGRAM GŁÓWNY ---
int main() {
    int shm_id = shmget(SHM_KEY, sizeof(struct GameMemory) , IPC_CREAT | 0640);
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0640);
    podnies(sem_id, 0);

    //Inicjalizacja 
    struct GameMemory *gra = (struct GameMemory*) shmat(shm_id, NULL, 0);

    inicjujProdukcje(gra);

    gra->gracze[0].surowce = 300;
    gra->gracze[0].lpiechota = 0;
    gra->gracze[0].cpiechota = 0;
    gra->gracze[0].jazda = 0;
    gra->gracze[0].robotnicy = 0;
    gra->gracze[0].iloscWygranychAtakow = 0;

    gra->gracze[1].surowce = 300;
    gra->gracze[1].lpiechota = 0;
    gra->gracze[1].cpiechota = 0;
    gra->gracze[1].jazda = 0;
    gra->gracze[1].robotnicy = 0;
    gra->gracze[1].iloscWygranychAtakow = 0;

    gra->gra_aktywna = 1;

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

    while(1) {
        // sprawdz czy gra jest aktywna
        if (gra->gra_aktywna == 0) {
            printf("Gra zakończona.\n");
            break;
        }

        // dodawanie surowców co turę
        if (sekunda == 100) {
            opusc(sem_id, 0);
            gra->gracze[0].surowce += 50 + (gra->gracze[0].robotnicy * 5);
            gra->gracze[1].surowce += 50 + (gra->gracze[1].robotnicy * 5);
            gra->gracze[0].zmianaStanuZasobow = 1;
            gra->gracze[1].zmianaStanuZasobow = 1;
            sekunda = 0;
            podnies(sem_id, 0);
        }

        if (ktoAtakuje != -1) {
            if (czasAtaku == 0) {
                printf("Atak rozpocznyna się!\n");
                symulacjaAtaku(gra, ktoAtakuje);
                ktoAtakuje = -1;
                continue;
            }
            czasAtaku--;
            printf("Atak gracza %d za %d sekund(y).\n", ktoAtakuje, czasAtaku / 100);
        }
        //printf(".\n"); //test odpowiedzi programu
        fflush(stdout);

        for (int i = 0; i < 2; i++) {
            sprawdzCzyWygrana(gra, i);

            // --- OBSŁUGA PRODUKCJI JEDNOSTEK ---
            if(gra->gracze[i].produkcja[0].czyWolne == 1) {
                if(czasProdukcji[i] == 0) {
                    if (gra->gracze[i].produkcja[0].ilosc > 1) {
                        czasProdukcji[i] = gra->gracze[i].produkcja[0].czas_pozostaly;
                        aktualizujProdukcja(gra, i, sem_id);
                    } else {
                        aktualizujProdukcja(gra, i, sem_id);
                        przesunProdukcja(gra, i);
                        czasProdukcji[i] = -1;
                    }
                } else if (czasProdukcji[i] > 0) {
                    czasProdukcji[i]--;
                } else if (czasProdukcji[i] == -1) {
                    czasProdukcji[i] = gra->gracze[i].produkcja[0].czas_pozostaly;
                }
            }

            // --- OBSŁUGA KOMEND UŻYTKOWNIKA ---
            if(gra->gracze[i].komenda != CMD_BRAK) {
                opusc(sem_id, 0);
                printf("Gracz %d wydał komendę: %d\n", i, gra->gracze[i].komenda);
                int calkowityKoszt = 0;
                switch(gra->gracze[i].komenda) {
                    case CMD_KUP_ROBOTNIKA:
                        calkowityKoszt = gra->gracze[i].komendaIlosc * 150;
                        if (gra->gracze[i].surowce >= calkowityKoszt) {
                            gra->gracze[i].surowce -= calkowityKoszt;
                            dodajProdukcja(gra, i, CMD_KUP_ROBOTNIKA, gra->gracze[i].komendaIlosc, 200);
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Dodano do produkcji.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Sukces. Gracz %d ma teraz %d robotników (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                        } else {
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Za mało surowców na zakup robotnika.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                        }
                        break;
                    case CMD_KUP_LPIECHOTA:
                        calkowityKoszt = gra->gracze[i].komendaIlosc * 100;
                        if (gra->gracze[i].surowce >= calkowityKoszt) {
                            gra->gracze[i].surowce -= calkowityKoszt;
                            dodajProdukcja(gra, i, CMD_KUP_LPIECHOTA, gra->gracze[i].komendaIlosc, 100);
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Dodano do produkcji.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Sukces. Gracz %d ma teraz %d lekkiej piechoty (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                        } else {
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Za mało surowców na zakup lekkiej piechoty.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                        }
                        break;
                    case CMD_KUP_CPIECHOTA:
                        calkowityKoszt = gra->gracze[i].komendaIlosc * 250;
                        if (gra->gracze[i].surowce >= calkowityKoszt) {
                            gra->gracze[i].surowce -= calkowityKoszt;
                            dodajProdukcja(gra, i, CMD_KUP_CPIECHOTA, gra->gracze[i].komendaIlosc, 300);
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Dodano do produkcji.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Sukces. Gracz %d ma teraz %d ciężkiej piechoty (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                        } else {
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Za mało surowców na zakup ciężkiej piechoty.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                        }
                        break;
                    case CMD_KUP_JAZDA:
                        calkowityKoszt = gra->gracze[i].komendaIlosc * 550;
                        if (gra->gracze[i].surowce >= calkowityKoszt) {
                            gra->gracze[i].surowce -= calkowityKoszt;
                            dodajProdukcja(gra, i, CMD_KUP_JAZDA, gra->gracze[i].komendaIlosc, 500);
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Dodano do produkcji.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Sukces. Gracz %d ma teraz %d jazdy (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                        } else {
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Za mało surowców na zakup jazdy.");
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                        }
                        break;
                    case CMD_ATAK:
                        if (ktoAtakuje == -1) {
                            ktoAtakuje = i;
                            if (sprawdzCzyMozeAtakowac(gra, ktoAtakuje) == 0) {
                                strcpy(gra->gracze[i].komunikat, "[SERWER] Za mało jednostek do ataku.\n");
                                gra->gracze[i].czyNowyKomunikat = 1;
                                ktoAtakuje = -1;
                            } else if (sprawdzCzyMozeAtakowac(gra, ktoAtakuje) == 1) {
                                czasAtaku = 500; // czas ataku 5s
                                strcpy(gra->gracze[i].komunikat, "[SERWER] Przygotowanie do ataku 5s.\n");
                                gra->gracze[i].czyNowyKomunikat = 1;
                            }
                        } else {
                            strcpy(gra->gracze[i].komunikat, "[SERWER] Ty lub inny gracz już atakuje.\n");
                            gra->gracze[i].czyNowyKomunikat = 1;
                        }
                        break;
                    default:
                        printf("Nieznana komenda od gracza %d: %d\n", i, gra->gracze[i].komenda);
                        strcpy(gra->gracze[i].komunikat, "[SERWER] Nieznana komenda.\n");
                        gra->gracze[i].czyNowyKomunikat = 1;
                        break;
                }
                gra->gracze[i].komenda = CMD_BRAK;
                podnies(sem_id, 0);
            }
        }
        sekunda++;
        usleep(10000); // opóźnienie 0.01 sekundy
    }

    //zamykanie pamięci współdzielonej
    shmdt(gra);
    return 0;
}