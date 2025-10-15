#include <iostream>
#include <signal.h>
#include <time.h>

#define N 8 

int OZNAKA_CEKANJA[N];
int PRIORITET[N];
int TEKUCI_PRIORITET;

void obrada_prekida(int j) {
    printf("Poceo obradu prekida %d\n", j);
    for(int i = 1; i <= 5; i++) {
        printf("Obrada prekida %d: %d/5\n", j, i);
        sleep(1);
    }
    
    printf("Zavrsio obradu prekida %d\n", j);
}

void prekidna_rutina(int sig) {
    int i;
    time_t t;

    time(&t);
    printf("Prekidna rutina pozvana u %s", ctime(&t));

    printf("Upisi razinu prekida: ");
    scanf("%d", &i);

    if (i > 7 || i < 0) {
        printf("Greska! Nepostojeca razina prekida!\n");
        return;
    }

    OZNAKA_CEKANJA[i] = 1;

    int x;

    do {
        x = 0;
        for (int j = TEKUCI_PRIORITET + 1; j < 8; j++) {
            if(OZNAKA_CEKANJA[j] != 0) {
                x = j;
            }
        };

        if (x > 0) {
            OZNAKA_CEKANJA[x] = 0;
            PRIORITET[x] = TEKUCI_PRIORITET;
            TEKUCI_PRIORITET = x;

            sigrelse(SIGINT);
            obrada_prekida(x);
            sighold(SIGINT);
            
            TEKUCI_PRIORITET = PRIORITET[x];
        }

    } while(x > 0);
}

int main() {
    TEKUCI_PRIORITET = 0;
    sigset(SIGINT, prekidna_rutina);

    printf("Poceo osnovni program\n");
    
    for(int i = 1; i <= 10; i++) {
        printf("Glavni program %d/10\n", i);
        sleep(1);
    };

    printf("Zavrsio osnovni program\n");

    return 0;
}