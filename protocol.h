#ifndef PROTOCOL_H
#define PROTOCOL_H

// Nazwa pliku kolejki na dysku
#define FIFO_FILE "/tmp/kolejka_serwera"
#define CLIENT_0_FIFO_FILE "/tmp/kolejka_klienta_0"
#define CLIENT_1_FIFO_FILE "/tmp/kolejka_klienta_1"

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
struct pakiet{
    int idGracza; // 0 lub 1
    int komenda;  // CMD_KUP lub CMD_ATAK lub CMD_LOGIN lub CMD_START lub CMD_WYJSCIE
    int typJednostki; // KUP_LEKKA_PIECHOTA, KUP_CIEZKA_PIECHOTA, KUP_JAZDA, KUP_ROBOTNIK
    int ilosc; // ilość jednostek do kupienia

    // Dane do ataku
    int ileLP; // ilość lekkiej piechoty do ataku
    int ileCP; // ilość ciężkiej piechoty do ataku
    int ileJazdy; // ilość jazdy do ataku
};

struct pakietOdp{
    int typ; // BLAD, SUKCES, INFO, AKTUALIZACJA, CMD_START
    char komunikat[256];
    int surowce;
    int lpiechota;
    int cpiechota;
    int jazda;
    int robotnicy;
};

#endif