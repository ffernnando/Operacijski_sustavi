#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;

int br_gen = 0;       // broj generiranih brojeva
int gl_var;           // globalna varijabla za zadatak
bool kraj = false;    // signal za kraj generiranja

// Semafori
sem_t postavljen_broj;   // signalizira da je broj spreman
sem_t procitan_broj;     // signalizira da je broj pročitan


//pošalji kao argument polje koje sadrži m i n pa da moreš na kraju
//izvoditi for za svih m dretvi
void* f_generiraj(void* arg) {
  //int n = *(int*)arg;
  int* arg_polje = (int*)arg;

  cout << "Dretva koja generira zadatke pocela je s radom. Broj zadataka = " << arg_polje[1] << endl;

  while (br_gen < arg_polje[1]) {
      int x = rand() % 1000000000 + 1;
      gl_var = x;

      cout << "Generiran broj " << gl_var << endl;

      // Signaliziraj da je novi broj spreman
      sem_post(&postavljen_broj);

      // Čekaj da neka dretva pročita broj
      sem_wait(&procitan_broj);

      ++br_gen;
  }

  // obavijesti sve dretve da nema više zadataka
  kraj = true;
  // dozvoli svim dretvama koje čekaju da ispitaju if (kraj) break;
  // pa da se završe
  for (int i = 0; i < arg_polje[0]; ++i) sem_post(&postavljen_broj);

  return NULL;
}

void* f_racunaj(void* arg) {
  
  while (true) {
      // čekaj da se zgenerira broj
      sem_wait(&postavljen_broj);

      if (kraj) break;  // dretva generiraj signalizirala da nema više zadataka

      int temp = gl_var;
      cout << "Dretva " << pthread_self() << " preuzela zadatak " << temp << endl;

      // signaliziraj da je broj pročitan
      sem_post(&procitan_broj);
      
      // jako jako veliki broj kad se računa faktorijela velikih brojeva
      long long sum = 0;
      for (int i = 1; i <= temp; i++) {
        sum += i;
      }

      cout << "Dretva " << pthread_self() << " zadatak = " << temp << " zbroj = " << sum << endl;
  }

  return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
      cout << "Morate unijeti 2 argumenta: m i n!" << endl;
      return 0;
    }
    

    int m = atoi(argv[1]); // broj dretava potrošača
    int n = atoi(argv[2]); // broj zadataka
    int arg_polje[] = { m, n };

    cout << "m = " << m << " | n = " << n << endl;

    sem_init(&postavljen_broj, 0, 0);
    sem_init(&procitan_broj, 0, 0);

    pthread_t generiraj_id;
    pthread_t racunaj_id[m];

    pthread_create(&generiraj_id, NULL, f_generiraj, arg_polje);

    for (int i = 0; i < m; i++) {
      pthread_create(&racunaj_id[i], NULL, f_racunaj, NULL);
    }

    pthread_join(generiraj_id, NULL);
    for (int i = 0; i < m; i++) {
      pthread_join(racunaj_id[i], NULL);
    }

    sem_destroy(&postavljen_broj);
    sem_destroy(&procitan_broj);

    return 0;
}