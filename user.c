#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include "gameMemory.h"
#include "protocol.h"
#include <string.h>
#include <unistd.h>
// --- ZMIEŃ NA STRING NA INT ---
int zmienNaInt(char *str) {
    int wynik = 0;
    for (int i = 0; str[i] == ' '; i++) {
        // pomiń spacje na początku
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1; // nieprawidłowy znak
        }
        wynik = wynik * 10 + (str[i] - '0');
    }
    return wynik;
}

// --- PROGRAM GŁÓWNY ---
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Użycie: %s <id_gracza>\n", argv[0]);
        return 1;
    }
    if (argv[1][0] < '0' || argv[1][0] > '1') {
        printf("Nieprawidłowy id_gracza. Dozwolone wartości: 0 lub 1.\n");
        return 1;
    }
    int id_gracza = argv[1][0] - '0';


    // --- INICJALIZACJA KOLEJEK KOMUNIKATÓW ---
    // SERWER
    int fd_serwer = open(FIFO_FILE, O_WRONLY);
    if (fd_serwer == -1) {
        perror("[BŁĄD] Brak połączenia z serwerem gry.\n");
        return 1;
    }
    // KLIENT
    char *my_fifo;
    if (id_gracza == 0) {
        my_fifo = CLIENT_0_FIFO_FILE;
    } else {
        my_fifo = CLIENT_1_FIFO_FILE;
    }
    mkfifo(my_fifo, 0666);
    int fd_klient = open(my_fifo, O_RDWR);
    if (fd_klient == -1) {
        perror("[BŁĄD] Nie można otworzyć kolejki klienta.\n");
        return 1;
    }
    
    // --- LOGOWANIE DO GRY ---
    struct pakiet komendy;
    komendy.idGracza = id_gracza;
    komendy.komenda = CMD_LOGIN;
    int czyWyslano = write(fd_serwer, &komendy, sizeof(komendy));
    if (czyWyslano == -1) {
        perror("[BŁĄD] Nie udało się wysłać komendy logowania do serwera.\n");
        return 1;
    }

    printf("[INICJALIZACJA] Zalogowano gracza %d do gry.\n", id_gracza);
    printf("[INICJALIZACJA] Oczekiwanie na drugiego gracza.\n");

    struct pakietOdp odpowiedz;
    while(1) {
        int bajty = read(fd_klient, &odpowiedz, sizeof(odpowiedz));
        if (bajty > 0) {
            if (odpowiedz.typ == CMD_START) {
                printf("[INICJALIZACJA] %s\n", odpowiedz.komunikat);
                break;
            }
        usleep(100000); // opóźnienie 0.1 sekundy
        }
    }

    // --- GŁÓWNA PĘTLA PROGRAMU ---
    pid_t pid = fork();
    if (pid == 0) {
        // Proces potomny - nasłuchiwanie komunikatów od serwera
        while(1) {
            struct pakietOdp odpowiedz;
            int bajtyOdp = read(fd_klient, &odpowiedz, sizeof(odpowiedz));
            if (bajtyOdp > 0) {
                switch (odpowiedz.typ) {
                    case INFO:
                        printf("[INFO] %s\n", odpowiedz.komunikat);
                        break;
                    case BLAD:
                        printf("[BŁĄD] %s\n", odpowiedz.komunikat);
                        break;
                    case SUKCES:
                        printf("[SUKCES] %s\n", odpowiedz.komunikat);
                        break;
                    case AKTUALIZACJA:
                        printf("[AKTUALIZACJA] Surowce: %d, Lekkiej piechoty: %d, Ciężkiej piechoty: %d, Jazdy: %d, Robotników: %d\n",
                               odpowiedz.surowce,
                               odpowiedz.lpiechota,
                               odpowiedz.cpiechota,
                               odpowiedz.jazda,
                               odpowiedz.robotnicy);
                        break;
                    default:
                        printf("[BŁĄD] Nieznany typ odpowiedzi od serwera: %d\n", odpowiedz.typ);
                        break;
                }
            }
            usleep(100000); // opóźnienie 0.1 sekundy
        }
    } else {
        // Proces macierzysty - wysyłanie komend do serwera
        while(1) {
            // Wczytywanie komendy od użytkownika
            struct pakiet wysylany;
            char command[50];
            int bajty = read(0, command, sizeof(command) - 1);
            if (bajty > 0) {
                command[bajty - 1] = '\0'; // Usuń znak nowej linii
            } else {
                continue; // Nic nie wpisano
            }
            wysylany.idGracza = id_gracza;

            // Przetwarzanie komendy
            if (strcmp(command, "help", 4) == 0) {
                printf("Dostępne komendy:\n");
                printf("kup - kupuje określoną ilość jednostek danego typu\n");
                printf("atak - atakuje drugiego gracza całą dostępną armią\n");
                printf("help - wyświetla pomoc\n");
                printf("wyjscie - kończy program\n");
                continue;
            } else if (strncmp(command, "kup", 3) == 0) {
                char coKupic[20];
                char charIleKupic[20];
                printf("Co chcesz kupić? (lpiechota, cpiechota, jazda, robotnik) \n");
                int bajty = read(0, coKupic, sizeof(coKupic) - 1);
                if (bajty > 0) {
                    coKupic[bajty - 1] = '\0';
                } else {
                    printf("Nieprawidłowa komenda. Możliwe do wyboru: lpiechota, cpiechota, jazda, robotnik\n");
                    continue;
                }
                printf("Ile chcesz kupić? \n");
                int bajty = read(0, charIleKupic, sizeof(charIleKupic) - 1);
                if (bajty > 0) {
                    charIleKupic[bajty - 1] = '\0';
                } else {
                    printf("Nieprawidłowa komenda. Podaj ilość jednostek do kupienia np. 3\n");
                    continue;
                }

                wysylany.komenda = CMD_KUP;
                if (strcmp(coKupic, "lpiechota") == 0) {
                    wysylany.typJednostki = KUP_LEKKA_PIECHOTA;
                } else if (strcmp(coKupic, "cpiechota") == 0) {
                    wysylany.typJednostki = KUP_CIEZKA_PIECHOTA;
                } else if (strcmp(coKupic, "jazda") == 0) {
                    wysylany.typJednostki = KUP_JAZDA;
                } else if (strcmp(coKupic, "robotnik") == 0) {
                    wysylany.typJednostki = KUP_ROBOTNIK;
                }
                wysylany.ilosc = zmienNaInt(charIleKupic);

                int czyWyslano = write(fd_serwer, &wysylany, sizeof(wysylany));
                if (czyWyslano == -1) {
                    perror("[BŁĄD] Nie udało się wysłać komendy do serwera.\n");
                }

            } else if (strncmp(command, "atak", 4) == 0) {
                char charIleLP[20];
                char charIleCP[20];
                char charIleJazdy[20];
                printf("Ile lekkiej piechoty wysłać do ataku? \n");
                int bajty = read(0, charIleLP, sizeof(charIleLP) - 1);
                if (bajty > 0) {
                    charIleLP[bajty - 1] = '\0';
                } else {
                    strcpy(charIleLP, "0");
                }
                printf("Ile ciężkiej piechoty wysłać do ataku? \n");
                int bajty = read(0, charIleCP, sizeof(charIleCP) - 1);
                if (bajty > 0) {
                    charIleCP[bajty - 1] = '\0';
                } else {
                    strcpy(charIleCP, "0");
                }
                printf("Ile jazdy wysłać do ataku? \n");
                int bajty = read(0, charIleJazdy, sizeof(charIleJazdy) - 1);
                if (bajty > 0) {
                    charIleJazdy[bajty - 1] = '\0';
                } else {
                    strcpy(charIleJazdy, "0");
                }
                wysylany.komenda = CMD_ATAK;
                wysylany.ileLP = zmienNaInt(charIleLP);
                wysylany.ileCP = zmienNaInt(charIleCP);
                wysylany.ileJazdy = zmienNaInt(charIleJazdy);

                int czyWyslano = write(fd_serwer, &wysylany, sizeof(wysylany));
                if (czyWyslano == -1) {
                    perror("[BŁĄD] Nie udało się wysłać komendy do serwera.\n");
                }

            } else if (strncmp(command, "wyjscie", 7) == 0) {
                printf("Zakończenie programu gracza %d.\n", id_gracza);
                break;
            } else {
                printf("Nieznana komenda. Wpisz 'help' aby uzyskać pomoc.\n");
                continue;
            }
        }
    }

    // Czyszczenie zasobów przed zakończeniem programu
    close(fd_serwer);
    close(fd_klient);
    unlink(my_fifo);
    return 0;
}