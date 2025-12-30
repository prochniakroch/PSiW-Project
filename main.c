#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>

int main() {
    int shm_id = shmget(123, 1 , IPC_CREAT | 0640);
    int surowce = 300;
    int lpiechota = 0; //100 1A 1.2O 2s
    int cpiechota = 0; //250 1.5A 3O 3s
    int jazda = 0; //550 3.5A 1.2O 5s
    int robotnicy = 0; //150 0A 0O 2s
    printf("Hello, World!\n");
    return 0;

    while(1) {
        sleep(1);
        surowce = surowce + 50 + (robotnicy * 5);
    }
}