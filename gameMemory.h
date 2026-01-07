#define SHM_KEY 123
#define MAX_GRACZY 2

// --- Definicje Komend ---
#define CMD_BRAK 0          // Pusto (serwer czeka)
#define CMD_KUP_LPIECHOTA 1
#define CMD_KUP_CPIECHOTA 2
#define CMD_KUP_JAZDA 3
#define CMD_KUP_ROBOTNIKA 4
#define CMD_ATAK 5

typedef struct {
    int surowce;
    int lpiechota;
    int cpiechota;
    int jazda;
    int robotnicy;
    int komenda;
} ZasobyGracza;

struct GameMemory {
    int gra_aktywna;
    ZasobyGracza gracze[MAX_GRACZY];
};