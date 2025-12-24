#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

using namespace std;


/* STRUKTURE PODATAKA S POČETNIM VRIJEDNOSTIMA
  M - varijabla zaključavanja (monitorski semafor)
  2 reda uvjeta uv[2] -> red_mik, red_lin
  br[2] = { 0, 0 } - dva brojača koliko programera je u restoranu
  čeka[2] = { 0, 0 } - dva brojača koliko programera čeka
  SITI = 0; - brojač koliko se programera jedne vrste najelo
*/

/* 
  m-funkcija udji(vrsta) {
    mutex_lock(&M);

    čeka[vrsta]++;

    dok je (br[1-vrsta] > 0 || (SITI >= N && (čeka[1-vrsta] > 0)))
      cond_wait(&uv[vrsta], &M);

    br[vrsta]++;
    čeka[vrsta]--;

    ako je (čeka[1-vrsta] > 0) SITI++;

    mutex_unlock(&M);
  }

  m-funckija izadji(vrsta) {
    mutex_lock(&M);

    br[vrsta]--;

    ako je(br[vrsta] == 0) {
      SITI = 0;
      cond_broadcast(&uv[1-vrsta]);
    }

    mutex_unlock(&M);
  }
*/

pthread_mutex_t M; // monitorski semafor
pthread_cond_t uv[2]; // 2 reda uvjeta
int N, x; // broj programer koji mogu ući u restoran dok drugi čekaju

int br[2] = { 0, 0 }; // dva brojača koliko je programera u restoranu
// int vrsta = 0; //0 (MS) ili 1 (Linux)
int ceka[2] = { 0, 0 }; // dva brojača koliko programera čeka
int SITI = 0; // brojač koliko se programera jedne vrste najelo

void ispis(int vrsta) {
    // kopije brojača da se ne dira stvarno stanje
    int temp_ms = ceka[0], temp_lin = ceka[1];

    cout << "Red Linux:";
    for (int i = 0; i < x; i++) {
        if (temp_lin > 0) { cout << "L"; temp_lin--; }
        else cout << "-";
    }
    cout << " Red Microsoft:";
    for (int i = 0; i < x; i++) {
        if (temp_ms > 0) { cout << "M"; temp_ms--; }
        else cout << "-";
    }

    cout << " Restoran:";
    for (int i = 0; i < br[1]; i++) cout << "L";
    for (int i = 0; i < br[0]; i++) cout << "M";

    cout << " --> ";
    if (vrsta == 0) cout << "M";
    else cout << "L";
}

void udji(int vrsta) {
  pthread_mutex_lock(&M);

  ceka[vrsta]++;

  while (br[1 - vrsta] > 0 || ((SITI >= N) && (ceka[1 - vrsta] > 0))) {
    // ispis stanja u restoranu i redovima
    ispis(vrsta);
    cout << " u red cekanja" << endl;
    pthread_cond_wait(&uv[vrsta], &M);
  }

  br[vrsta]++;
  ceka[vrsta]--;
  
  if (ceka[1 - vrsta] > 0) {
    SITI++;
  }

  // ispis stanja u restoranu i redovima
  ispis(vrsta);
  cout << " u restoran" << endl;
  pthread_mutex_unlock(&M);
};

void izadji(int vrsta) {
  pthread_mutex_lock(&M);
  br[vrsta]--;
  
  if (br[vrsta] == 0) {
    SITI = 0;
    pthread_cond_broadcast(&uv[1 - vrsta]);
  }

  ispis(vrsta);
  cout << " iz restorana" << endl;
  pthread_mutex_unlock(&M);
};

/*
programer(vrsta) {
  programiraj;
  
  udi(vrsta); -> monitor? vrsta = red?

  jedi;

  izađi(vrsta); -> monitor?
}
*/

void* programer(void* arg) {
  int vrsta = *(int*)arg;
  // sleep nešto
  usleep(rand() % 100000);
  udji(vrsta);

  // sleep nešto
  usleep(rand() % 100000);
  izadji(vrsta);

  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout<<"GRESKA!\nPri pokretanju programa morate unijeti tocno 2 argumenta: N i M!"<<endl;
    exit(EXIT_FAILURE);
  }
  N = atoi(argv[1]);
  x = atoi(argv[2]);

  pthread_mutex_init(&M, NULL);
  pthread_cond_init(&uv[0], NULL);
  pthread_cond_init(&uv[1], NULL);

  pthread_t id_linux[x], id_MS[x];
  int arg_ms[x], arg_lin[x];
  for (int i = 0; i < x; i++) {
    arg_ms[i] = 0;
    arg_lin[i] = 1;
    pthread_create(&id_MS[i], NULL, programer, &arg_ms[i]); //microsoft
    pthread_create(&id_linux[i], NULL, programer, &arg_lin[i]); //linux
  }

  for (int i = 0; i < x; i++) {
    pthread_join(id_MS[i], NULL);
    pthread_join(id_linux[i], NULL);
  }

  pthread_mutex_destroy(&M);
  pthread_cond_destroy(&uv[0]);
  pthread_cond_destroy(&uv[1]);


  return 0;
};