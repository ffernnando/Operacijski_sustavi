#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

using namespace std;

// ---------------------------------------- GLOBALNI MUTEXI, UVJETI I VARIJABLE ----------------------------------------
pthread_mutex_t m;
pthread_cond_t red;

int br = 0;
int N;

/* PSEUDOKOD iz knjige
  m-funkcija Barijera(m, red) {
    Zaključati_monitor(m);
    br++;
    ako je(br < N) {
      Čekati_u_redu(red, m);
    } inače {
      br = 0;
      Propustiti_sve_iz_reda(red); 
    }
    Otključati_monitor(m);
  }
*/

// ---------------------------------------- FUNKCIJA BARIJERA ----------------------------------------
void* barijera(void* arg) {
  int* n = (int*) arg;
  int x;

  pthread_mutex_lock(&m); // Zaključati_monitor(m)
  cout << "Dretva " << *n << ". unesite broj" << endl;
  cin >> x;
  br++;
  if (br < N) {
    pthread_cond_wait(&red, &m); // Čekati_u_redu(red, m)
  } else {
    br = 0;
    pthread_cond_broadcast(&red); // Propustiti_sve_iz_reda(red)
  }
  cout << "Dretva " << *n << ". uneseni broj je " << x << endl;
  pthread_mutex_unlock(&m);
  
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cout<<"GRESKA!\nPri pokretanju programa morate unijeti tocno 1 argument: N!"<<endl;
    exit(EXIT_FAILURE);
  }

  N = atoi(argv[1]);
  cout << "Broj dretava = " << N << endl;

  // Inicijalizacija mutexa i uvjeta
  pthread_mutex_init(&m, NULL);
  pthread_cond_init(&red, NULL);

  pthread_t id_polje[N];
  int i_polje[N];

  for (int i = 0; i < N; i++) {
    i_polje[i] = i;
    pthread_create(&id_polje[i], NULL, barijera, &i_polje[i]);
  }

  for (int i = 0; i < N; i++) {
    pthread_join(id_polje[i], NULL);
  }

  // Brisanje mutexa i uvjeta
  pthread_mutex_destroy(&m);
  pthread_cond_destroy(&red);

  return 0;
};