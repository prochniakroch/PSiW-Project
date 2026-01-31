#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "protocol.h"
#include "gameMemory.h"

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
int czasAtaku[2] = {-1, -1};
int ktoAtakuje[2] = {0, 0};
int czasProdukcji[2] = {-1, -1};
int sem_id;
int calkowityKoszt;


// --- FUNKCJE ---
void sprawdzCzyWygrana(struct GameMemory *gra, int gracz){
    if (gra->gracze[gracz].iloscWygranychAtakow >= 5) {
        podnies(sem_id, 0);
        printf("Gracz %d wygrał grę!\n", gracz);
        if (gracz == 0) {
            strcpy(gra->gracze[0].komunikat, "Wygrałeś grę!\n");
            strcpy(gra->gracze[1].komunikat, "Przegrałeś grę!\n");
        } else if (gracz == 1) {
            strcpy(gra->gracze[1].komunikat, "Wygrałeś grę!\n");
            strcpy(gra->gracze[0].komunikat, "Przegrałeś grę!\n");

        }
        gra->gracze[0].jakiKomunikat = INFO;
        gra->gracze[1].jakiKomunikat = INFO;
        gra->gracze[0].czyNowyKomunikat = 1;
        gra->gracze[1].czyNowyKomunikat = 1;
        opusc(sem_id, 0);
        gra->gra_aktywna = 0;
    }
}

void symulacjaAtaku(struct GameMemory *gra, int graczAtakujacy) {
    int graczObraniajacy;
    if (graczAtakujacy == 0) {
        printf("[ATAK] Gracz 0 atakuje gracza 1!\n");
        graczObraniajacy = 1;
    } else {
        printf("[ATAK] Gracz 1 atakuje gracza 0!\n");
        graczObraniajacy = 0;
    }
    
    // Sila Ataku i Sila Obrony obu graczy
    int silaAtaku0 = gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota * 1 + gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota * 1.5 + gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda * 3.5;
    int silaObrony0 = gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota * 1.2 + gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota * 3 + gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda * 1.2;
    int silaAtaku1 = gra->gracze[graczObraniajacy].lpiechota * 1 + gra->gracze[graczObraniajacy].cpiechota * 1.5 + gra->gracze[graczObraniajacy].jazda * 3.5;
    int silaObrony1 = gra->gracze[graczObraniajacy].lpiechota * 1.2 + gra->gracze[graczObraniajacy].cpiechota * 3 + gra->gracze[graczObraniajacy].jazda * 1.2;
    printf("[ATAK] Siła ataku: %d vs Siła obrony: %d\n", silaAtaku0, silaObrony1);

    if (silaObrony1 == 0) {
        printf("Obrońca nie ma jednostek do obrony! Wszystkie jednostki atakującego przechodzą bez strat.\n");
        strcpy(gra->gracze[graczAtakujacy].komunikat, "Wszystkie jednostki przeciwnika zostały zniszczone!\n");
        gra->gracze[graczAtakujacy].jakiKomunikat = SUKCES;
        gra->gracze[graczAtakujacy].czyNowyKomunikat = 1;
        gra->gracze[graczAtakujacy].iloscWygranychAtakow += 1;
        return;
    }

    if (silaAtaku0 - silaObrony1 > 0) {
        // Sila Ataku - Sila Obrony > 0
        printf("Wszystkie jednostki obrońcy zostały zniszczone!\n");
        int stratyAtakujacyLP = gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota * (silaAtaku0 / silaObrony1);
        int stratyAtakujacyCP = gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota * (silaAtaku0 / silaObrony1);
        int stratyAtakujacyJazda = gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda * (silaAtaku0 / silaObrony1);

        gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota -= stratyAtakujacyLP;
        gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota -= stratyAtakujacyCP;
        gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda -= stratyAtakujacyJazda;

        gra->gracze[graczObraniajacy].lpiechota = 0;
        gra->gracze[graczObraniajacy].cpiechota = 0;
        gra->gracze[graczObraniajacy].jazda = 0;
        gra->gracze[graczAtakujacy].iloscWygranychAtakow += 1;
    } else {
        // Sila Ataku < Sila Obrony
        // X * (silaAtaku / silaObrony)
        int stratyObronyLP = gra->gracze[graczObraniajacy].lpiechota * (silaAtaku0 / silaObrony1);
        int stratyObronyCP = gra->gracze[graczObraniajacy].cpiechota * (silaAtaku0 / silaObrony1);
        int stratyObronyJazda = gra->gracze[graczObraniajacy].jazda * (silaAtaku0 / silaObrony1);
        
        int stratyAtakujacyLP = gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota * (silaAtaku1 / silaObrony0);
        int stratyAtakujacyCP = gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota * (silaAtaku1 / silaObrony0);
        int stratyAtakujacyJazda = gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda * (silaAtaku1 / silaObrony0);

        gra->gracze[graczObraniajacy].lpiechota -= stratyObronyLP;
        gra->gracze[graczObraniajacy].cpiechota -= stratyObronyCP;
        gra->gracze[graczObraniajacy].jazda -= stratyObronyJazda;

        gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota -= stratyAtakujacyLP;
        gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota -= stratyAtakujacyCP;
        gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda -= stratyAtakujacyJazda;
        strcpy(gra->gracze[graczAtakujacy].komunikat, "Twój atak został odparty!\n");
        gra->gracze[graczAtakujacy].jakiKomunikat = INFO;
        gra->gracze[graczAtakujacy].czyNowyKomunikat = 1;
        strcpy(gra->gracze[graczObraniajacy].komunikat, "Obroniłeś atak przeciwnika!\n");
        gra->gracze[graczObraniajacy].jakiKomunikat = INFO;
        gra->gracze[graczObraniajacy].czyNowyKomunikat = 1;
    }
}


void doAtaku(struct GameMemory *gra, int graczAtakujacy, int liczbaLP, int liczbaCP, int liczbaJazdy) {
    gra->gracze[graczAtakujacy].lpiechota -= liczbaLP;
    gra->gracze[graczAtakujacy].cpiechota -= liczbaCP;
    gra->gracze[graczAtakujacy].jazda -= liczbaJazdy;
    gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota = liczbaLP;
    gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota = liczbaCP;
    gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda = liczbaJazdy;
}

void poAtaku(struct GameMemory *gra, int graczAtakujacy, int liczbaLP, int liczbaCP, int liczbaJazdy) {
    gra->gracze[graczAtakujacy].wTrakcieAtaku.lpiechota = 0;
    gra->gracze[graczAtakujacy].wTrakcieAtaku.cpiechota = 0;
    gra->gracze[graczAtakujacy].wTrakcieAtaku.jazda = 0;
    gra->gracze[graczAtakujacy].lpiechota += liczbaLP;
    gra->gracze[graczAtakujacy].cpiechota += liczbaCP;
    gra->gracze[graczAtakujacy].jazda += liczbaJazdy;
}

int czyMozeAtakowac(struct GameMemory *gra, int gracz) {
    if (gra->gracze[gracz].wTrakcieAtaku.lpiechota > 0 || gra->gracze[gracz].wTrakcieAtaku.cpiechota > 0 || gra->gracze[gracz].wTrakcieAtaku.jazda > 0) {
        return 1;
    } else {
        return 0;
    }
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
        strcpy(gra->gracze[gracz].komunikat, "Kolejka pełna jednak nie dodaliśmyy twojego zamowienia. Zwrocilismy koszt zamowienia.\n");
        gra->gracze[gracz].surowce =+ calkowityKoszt;
        gra->gracze[gracz].jakiKomunikat = BLAD;
        gra->gracze[gracz].czyNowyKomunikat = 1;
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


// --- PRZESYŁANIE KOMUNIKATÓW DO GRACZY ---
void wyslijKomunikatDoGracza(struct GameMemory *gra, int gracz, int fd_klient0, int fd_klient1) {
    struct pakietOdp nowyKomunikat;
    char komunikat[256];
    komunikat[0] = '\0';
    opusc(sem_id, 0);
    int rodzaj = gra->gracze[gracz].jakiKomunikat;
    strcpy(komunikat, gra->gracze[gracz].komunikat);
    podnies(sem_id, 0);
    if (gracz == 0) {
        nowyKomunikat.typ = rodzaj;
        strcpy(nowyKomunikat.komunikat, komunikat);
        write(fd_klient0, &nowyKomunikat, sizeof(nowyKomunikat));
    } else if (gracz == 1) {
        nowyKomunikat.typ = rodzaj;
        strcpy(nowyKomunikat.komunikat, komunikat);
        write(fd_klient1, &nowyKomunikat, sizeof(nowyKomunikat));
    }
}

// --- PROGRAM GŁÓWNY ---
int main() {
    // Inicjalizacja pamięci współdzielonej i semaforów
    int shm_id = shmget(SHM_KEY, sizeof(struct GameMemory) , IPC_CREAT | 0640);
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0640);
    podnies(sem_id, 0);

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

    printf("[INICJALIZACJA] Pamięc gotowa.\n");

    // Inicjalizacja kolejek komunikatów (1/2)
    // SERWER
    unlink(FIFO_FILE);
    mkfifo(FIFO_FILE, 0666);
    int fd_serwer = open(FIFO_FILE, O_RDWR);
    if (fd_serwer == -1) {
        perror("[BŁĄD] Nie można utworzyć kolejki serwera.\n");
        return 1;
    }
    struct pakiet komendy;

    // --- OCZEKIWANIE NA GRACZY ---
    printf("[INICJALIZACJA] Oczekiwanie na graczy.\n");
    int graczeGotowi[2] = {0, 0};
    int liczbaGraczy = 0;

    while (liczbaGraczy < 2) {
        int bajty = read(fd_serwer, &komendy, sizeof(komendy));
        if (bajty > 0) {
            if (komendy.komenda == CMD_LOGIN && graczeGotowi[komendy.idGracza] == 0) {
                graczeGotowi[komendy.idGracza] = 1;
                liczbaGraczy++;
                printf("[INICJALIZACJA] Gracz %d dołączył do gry. (%d/2)\n", komendy.idGracza, liczbaGraczy);
            }
        }
        usleep(100000); // opóźnienie 0.1 sekundy
    }
    printf("[INICJALIZACJA] Wszyscy dołączyli.");

    // Inicjalizacja kolejek komunikatów (2/2)
    // KLIENT
    int fd_klient0 = open(CLIENT_0_FIFO_FILE, O_WRONLY);
    if (fd_klient0 == -1) {
        perror("[BŁĄD] Brak komunakcji z klientem 0.\n");
        return 1;
    }

    int fd_klient1 = open(CLIENT_1_FIFO_FILE, O_WRONLY);
    if (fd_klient1 == -1) {
        perror("[BŁĄD] Brak komunakcji z klientem 1.\n");
        return 1;
    }

    struct pakietOdp start;
    start.typ = CMD_START;
    strcpy(start.komunikat, "[SERWER] Gra się rozpoczyna!\n");
    write(fd_klient0, &start, sizeof(start));
    write(fd_klient1, &start, sizeof(start));

    printf("[INICJALIZACJA] Kolejki komunikatów gotowe.\n");

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
        pid_t pid = fork();
        if (pid == 0) {
            // Proces potomny - nasłuchiwanie komunikatów od klientow
            while(1) {
                struct pakiet komendy;
                int bajty = read(fd_serwer, &komendy, sizeof(komendy));
                if (bajty > 0) {
                    int idGracza = komendy.idGracza;
                    printf("[KOMUNIKAT] Otrzymano komendę %d od gracza %d.\n", komendy.komenda, idGracza);
                    opusc(sem_id, 0);
                    gra->gracze[idGracza].komenda = komendy.komenda;
                    if (komendy.komenda == CMD_KUP) {
                        gra->gracze[idGracza].komendaTyp = komendy.typJednostki;
                        gra->gracze[idGracza].komendaIlosc = komendy.ilosc;
                    } else if (komendy.komenda == CMD_ATAK) {
                        gra->gracze[idGracza].wTrakcieAtaku.lpiechota = komendy.ileLP;
                        gra->gracze[idGracza].wTrakcieAtaku.cpiechota = komendy.ileCP;
                        gra->gracze[idGracza].wTrakcieAtaku.jazda = komendy.ileJazdy;
                        gra->gracze[idGracza].czyAtakuje = 1;
                    }
                    podnies(sem_id, 0);
                }
                usleep(100000); // opóźnienie 0.1 sekundy
            }
            exit(0);
        } else {
        // Proces rodzicielski - główna pętla gry
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
            gra->gracze[0].zmianaStanu = 1;
            gra->gracze[1].zmianaStanu = 1;
            podnies(sem_id, 0);
            sekunda = 0;
        }

        // wysyłanie aktualizacji do klientów
        if (gra->gracze[0].zmianaStanu == 1) {
            struct pakietOdp aktualizacja;
            opusc(sem_id, 0);
            aktualizacja.typ = AKTUALIZACJA;
            aktualizacja.surowce = gra->gracze[0].surowce;
            aktualizacja.lpiechota = gra->gracze[0].lpiechota;
            aktualizacja.cpiechota = gra->gracze[0].cpiechota;
            aktualizacja.jazda = gra->gracze[0].jazda;
            aktualizacja.robotnicy = gra->gracze[0].robotnicy;
            gra->gracze[0].zmianaStanu = 0;
            podnies(sem_id, 0);
            write(fd_klient0, &aktualizacja, sizeof(aktualizacja));
        }    
        if (gra->gracze[1].zmianaStanu == 1) {
            struct pakietOdp aktualizacja;
            opusc(sem_id, 0);
            aktualizacja.typ = AKTUALIZACJA;
            aktualizacja.surowce = gra->gracze[1].surowce;
            aktualizacja.lpiechota = gra->gracze[1].lpiechota;
            aktualizacja.cpiechota = gra->gracze[1].cpiechota;
            aktualizacja.jazda = gra->gracze[1].jazda;
            aktualizacja.robotnicy = gra->gracze[1].robotnicy;
            gra->gracze[1].zmianaStanu = 0;
            podnies(sem_id, 0);
            write(fd_klient1, &aktualizacja, sizeof(aktualizacja));   
        }
        if (gra->gracze[0].czyNowyKomunikat == 1) {
            wyslijKomunikatDoGracza(gra, 0, fd_klient0, fd_klient1);
            opusc(sem_id, 0);
            gra->gracze[0].czyNowyKomunikat = 0;
            podnies(sem_id, 0);
        }
        if (gra->gracze[1].czyNowyKomunikat == 1) {
            wyslijKomunikatDoGracza(gra, 1, fd_klient0, fd_klient1);
            opusc(sem_id, 0);
            gra->gracze[1].czyNowyKomunikat = 0;
            podnies(sem_id, 0);
        }

        //printf(".\n"); //test odpowiedzi programu
        fflush(stdout);

        for (int i = 0; i < 2; i++) {
            sprawdzCzyWygrana(gra, i);

            if(ktoAtakuje[i] == 1) {
                if (czasAtaku[i] == 0) {
                    printf("Gracz %d atakuje!\n", i);
                    symulacjaAtaku(gra, i);
                    ktoAtakuje[i] = 0;
                    czasAtaku[i] = -1;
                } else {
                    czasAtaku[i]--;
                }
            }

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
                switch(gra->gracze[i].komenda) {
                    case CMD_KUP:
                        switch(gra->gracze[i].komendaTyp) {
                            case CMD_KUP_ROBOTNIKA:
                                calkowityKoszt = gra->gracze[i].komendaIlosc * 150;
                                if (gra->gracze[i].surowce >= calkowityKoszt) {
                                    gra->gracze[i].surowce -= calkowityKoszt;
                                    dodajProdukcja(gra, i, CMD_KUP_ROBOTNIKA, gra->gracze[i].komendaIlosc, 200);
                                    strcpy(gra->gracze[i].komunikat, "Dodano do produkcji.");
                                    gra->gracze[i].jakiKomunikat = SUKCES;                                    
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    printf("-> Sukces. Gracz %d ma teraz %d robotników (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                                } else {
                                    strcpy(gra->gracze[i].komunikat, "Za mało surowców na zakup robotnika.");
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    gra->gracze[i].jakiKomunikat = BLAD;
                                    printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                                }
                                break;
                            case CMD_KUP_LPIECHOTA:
                                calkowityKoszt = gra->gracze[i].komendaIlosc * 100;
                                if (gra->gracze[i].surowce >= calkowityKoszt) {
                                    gra->gracze[i].surowce -= calkowityKoszt;
                                    dodajProdukcja(gra, i, CMD_KUP_LPIECHOTA, gra->gracze[i].komendaIlosc, 100);
                                    strcpy(gra->gracze[i].komunikat, "Dodano do produkcji.");
                                    gra->gracze[i].jakiKomunikat = SUKCES;
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    printf("-> Sukces. Gracz %d ma teraz %d lekkiej piechoty (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                                } else {
                                    strcpy(gra->gracze[i].komunikat, "Za mało surowców na zakup lekkiej piechoty.");
                                    gra->gracze[i].jakiKomunikat = BLAD;
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                                }
                                break;
                            case CMD_KUP_CPIECHOTA:
                                calkowityKoszt = gra->gracze[i].komendaIlosc * 250;
                                if (gra->gracze[i].surowce >= calkowityKoszt) {
                                    gra->gracze[i].surowce -= calkowityKoszt;
                                    dodajProdukcja(gra, i, CMD_KUP_CPIECHOTA, gra->gracze[i].komendaIlosc, 300);
                                    strcpy(gra->gracze[i].komunikat, "Dodano do produkcji.");
                                    gra->gracze[i].jakiKomunikat = SUKCES;                                    
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    printf("-> Sukces. Gracz %d ma teraz %d ciężkiej piechoty (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                                } else {
                                    strcpy(gra->gracze[i].komunikat, "Za mało surowców na zakup ciężkiej piechoty.");
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    gra->gracze[i].jakiKomunikat = BLAD;
                                    printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                                }
                                break;
                            case CMD_KUP_JAZDA:
                                calkowityKoszt = gra->gracze[i].komendaIlosc * 550;
                                if (gra->gracze[i].surowce >= calkowityKoszt) {
                                    gra->gracze[i].surowce -= calkowityKoszt;
                                    dodajProdukcja(gra, i, CMD_KUP_JAZDA, gra->gracze[i].komendaIlosc, 500);
                                    strcpy(gra->gracze[i].komunikat, "Dodano do produkcji.");
                                    gra->gracze[i].jakiKomunikat = SUKCES;
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    printf("-> Sukces. Gracz %d ma teraz %d jazdy (w produkcji).\n", i, gra->gracze[i].komendaIlosc);
                                } else {
                                    strcpy(gra->gracze[i].komunikat, "Za mało surowców na zakup jazdy.");
                                    gra->gracze[i].jakiKomunikat = BLAD;
                                    gra->gracze[i].czyNowyKomunikat = 1;
                                    printf("-> Błąd. Gracz %d ma za mało surowców!\n", i);
                                }
                                break;
                            }
                        break;
                    case CMD_ATAK:
                        if (gra->gracze[i].czyAtakuje == 1) {
                            strcpy(gra->gracze[i].komunikat, "Już przygotowujesz atak.\n");
                            gra->gracze[i].jakiKomunikat = BLAD;
                            gra->gracze[i].czyNowyKomunikat = 1;
                            printf("Gracz %d już przygotowuje atak!\n", i);
                        } else {
                            if (czyMozeAtakowac(gra, i)) {
                                gra->gracze[i].czyAtakuje = 1;
                                czasAtaku = 500; // czas do rozpoczęcia ataku (5 sekund)
                                strcpy(gra->gracze[i].komunikat, "Atak się rozpoczyna, potrwa 5 sekund.\n");
                                gra->gracze[i].jakiKomunikat = INFO;
                                gra->gracze[i].czyNowyKomunikat = 1;
                                printf("Gracz %d przygotowuje atak!\n", i);
                            } else {
                                strcpy(gra->gracze[i].komunikat, "Nie masz jednostek do ataku.\n");
                                gra->gracze[i].jakiKomunikat = BLAD;
                                gra->gracze[i].czyNowyKomunikat = 1;
                                printf("Gracz %d nie ma jednostek do ataku!\n", i);
                            }
                        }
                        break;
                    default:
                        printf("Nieznana komenda od gracza %d: %d\n", i, gra->gracze[i].komenda);
                        strcpy(gra->gracze[i].komunikat, "Nieznana komenda.");
                        gra->gracze[i].jakiKomunikat = BLAD;
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
    }

    //zamykanie pamięci współdzielonej
    close(fd_serwer);
    close(fd_klient0);
    close(fd_klient1);
    unlink(FIFO_FILE);
    shmdt(gra);
    return 0;
}