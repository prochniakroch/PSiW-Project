#define SHM_KEY 123
#define MAX_GRACZY 2

typedef struct {
    int surowce;
    int lpiechota;
    int cpiechota;
    int jazda;
    int robotnicy;
} ZasobyGracza;

struct GameMemory {
    int gra_aktywna;
    ZasobyGracza gracze[MAX_GRACZY];
};