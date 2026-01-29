#define SHM_KEY 123
#define SEM_KEY 456
#define MSG_KEY 420
#define MAX_GRACZY 2
#define MAX_PRODUKCJA 5

// --- Definicje Komend ---
#define CMD_BRAK 0          // Pusto (serwer czeka)
#define CMD_KUP_LPIECHOTA 1
#define CMD_KUP_CPIECHOTA 2
#define CMD_KUP_JAZDA 3
#define CMD_KUP_ROBOTNIKA 4
#define CMD_ATAK 5

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
    int komendaIlosc;
    int czyAtakuje;
    int czasAtaku;
    int iloscWygranychAtakow;
    char komunikat[100];
    int czyNowyKomunikat;
    int zmianaStanu;
    struct aktualneZadanie produkcja[MAX_PRODUKCJA];
    struct wTrakcieAtaku wTrakcieAtaku;
} ZasobyGracza;

struct GameMemory {
    int gra_aktywna;
    ZasobyGracza gracze[MAX_GRACZY];
};