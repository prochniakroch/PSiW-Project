#define SHM_KEY 123
#define SEM_KEY 456
#define MSG_KEY 420
#define MAX_GRACZY 2
#define MAX_PRODUKCJA 5

// --- Definicje Komend ---
#define CMD_BRAK 0          // Pusto (serwer czeka)
#define CMD_KUP 1          // Kup jednostki
#define CMD_ATAK 2         // Atakuj przeciwnika

// --- Definicje Typów Jednostek ---
#define CMD_KUP_LPIECHOTA 1
#define CMD_KUP_CPIECHOTA 2
#define CMD_KUP_JAZDA 3
#define CMD_KUP_ROBOTNIKA 4

// --- TYPY ODPOWIEDZI ---
#define INFO 1
#define BLAD 2
#define SUKCES 3
#define AKTUALIZACJA 4

struct aktualneZadanie {
    int czyWolne;  // czy zadanie jest aktywne
    int typ_jednostki; // typ jednostki do wyprodukowania
    int ilosc;        // ilość jednostek do wyprodukowania
    int czas_pozostaly; // pozostały czas produkcji
};

struct wTrakcieAtaku {
    int lpiechota;
    int cpiechota;
    int jazda;
};

typedef struct {
    int surowce;
    int lpiechota;
    int cpiechota;
    int jazda;
    int robotnicy;
    int komenda;
    int komendaTyp;
    int komendaIlosc;
    int iloscWygranychAtakow;
    char komunikat[256];
    int jakiKomunikat;
    int czyNowyKomunikat;
    int zmianaStanu;
    struct aktualneZadanie produkcja[MAX_PRODUKCJA];
    struct wTrakcieAtaku wTrakcieAtaku;
} ZasobyGracza;

struct GameMemory {
    int gra_aktywna;
    ZasobyGracza gracze[MAX_GRACZY];
};