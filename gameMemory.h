#define SHM_KEY 123
#define SEM_KEY 456
#define MAX_GRACZY 2
#define MAX_PRODUKCJA 5

// --- DEFINICJE KOMEND ---
#define CMD_BRAK 0          // Pusto (serwer czeka)
#define CMD_KUP 1          // Kup jednostki
#define CMD_ATAK 2         // Atakuj przeciwnika

// --- DEFINICJE TYPÓW JEDNOSTEK ---
#define CMD_KUP_LPIECHOTA 1
#define CMD_KUP_CPIECHOTA 2
#define CMD_KUP_JAZDA 3
#define CMD_KUP_ROBOTNIKA 4

// --- TYPY ODPOWIEDZI ---
#define INFO 1
#define BLAD 2
#define SUKCES 3
#define AKTUALIZACJA 4

// --- PAMIĘĆ WSPÓŁDZIELONA ---
// Kolejka produkcji
struct aktualneZadanie {
    int czyWolne;  // czy zadanie jest aktywne
    int typ_jednostki; // typ jednostki do wyprodukowania
    int ilosc;        // ilość jednostek do wyprodukowania
    int czas_pozostaly; // pozostały czas produkcji
};

// Jednostki w trakcie ataku
struct wTrakcieAtaku {
    int lpiechota; // ilosc lekkiej piechoty 
    int cpiechota; // ilosc ciezkiej piechoty
    int jazda; // ilosc jazdy
};

// Aktualne dane gracza
typedef struct {
    int surowce;
    int lpiechota;
    int cpiechota;
    int jazda;
    int robotnicy;
    int komenda; // CMD_BRAK, CMD_KUP, CMD_ATAK
    int komendaTyp; // CMD_KUP_LPIECHOTA, CMD_KUP_CPIECHOTA, CMD_KUP_JAZDA, CMD_KUP_ROBOTNIKA
    int komendaIlosc; // ilosc kupionych jednostek
    int iloscWygranychAtakow; // ilosc wygranych atakow
    char komunikat[256]; // komunikat od serwera
    int jakiKomunikat; // INFO, BLAD, SUKCES, AKTUALIZACJA
    int czyNowyKomunikat; // 1 lub 0
    int zmianaStanu; // zmiana stanu surowcow
    struct aktualneZadanie produkcja[MAX_PRODUKCJA];
    struct wTrakcieAtaku wTrakcieAtaku;
} ZasobyGracza;

struct GameMemory {
    int gra_aktywna;
    ZasobyGracza gracze[MAX_GRACZY];
};