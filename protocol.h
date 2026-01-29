#ifndef PROTOCOL_H
#define PROTOCOL_H

// Nazwa pliku kolejki na dysku
#define FIFO_FILE "kolejka_serwera"
#define CLIENT_0_FIFO_FILE "kolejka_klienta_0"
#define CLIENT_1_FIFO_FILE "kolejka_klienta_1"

// --- TYPY KOMEND ---
#define CMD_KUP 1
#define CMD_ATAK 2
#define CMD_LOGIN 100
#define CMD_START 101
#define CMD_WYJSCIE 102

// --- TYPY JEDNOSTEK DO KUPIENIA ---
#define KUP_LEKKA_PIECHOTA 1
#define KUP_CIEZKA_PIECHOTA 2
#define KUP_JAZDA 3
#define KUP_ROBOTNIK 4

// --- TYPY ODPOWIEDZI ---
#define INFO 1
#define BLAD 2
#define SUKCES 3
#define AKTUALIZACJA 4

// Struktura danych wysyłanych w pakiecie
typedef struct {
    int idGracza; // 0 lub 1
    int komenda;  // 1=KUP, 2=ATAK
    int typJednostki; // 1=lekka piechota, 2=ciężka piechota, 3=jazda, 4=robotnik
    int ilosc; // ilość jednostek do kupienia

    // Dane do ataku
    int ileLP; // ilość lekkiej piechoty do ataku
    int ileCP; // ilość ciężkiej piechoty do ataku
    int ileJazdy; // ilość jazdy do ataku
} pakiet;

typedef struct {
    int typ; // 1 = INFO, 2 = BŁĄD, 3 = SUKCES, 4 = AKTUALIZACJA
    char komunikat[256];
} pakietOdp;

#endif