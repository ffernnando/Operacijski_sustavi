#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

using namespace std;

int* polje_brojeva;
int pravo = 0;

//pravo = 0 --> generiranje se smije izvoditi
void* generiranje(void* arg) {
  int* arg_polje = (int*)arg;
  
  for (int i= 0; i < arg_polje[1]; i++) {
    while (pravo == 1) {
      usleep(5000);
    }
      
    for (int i = 0; i < arg_polje[0]; i++) {

      polje_brojeva[i] = (int)(rand() % 100);
      cout<<"Polje_brojeva["<<i<<"]: "<<polje_brojeva[i]<<endl;
    }

    pravo = 1;
  }

  return NULL;
};

//pravo = 1 --> racunanje se smije izvoditi
void* racunanje(void* arg) {
  int* arg_polje = (int*)arg;
  
  for(int i = 0; i < arg_polje[1]; i++) {
    while (pravo == 0) {
      usleep(5000);
    }
    
    int suma = 0;
    for(int i = 0; i < arg_polje[0]; i++) {
      suma += polje_brojeva[i];
    }
    cout<<"Zbroj = "<<suma<<endl;

    pravo = 0;
  }

  return NULL;
};

int main(int argc, char* argv[]) {

pthread_t id_novi;
pthread_create(&id_novi, NULL, generiranje, NULL);


  if (argc != 3) {
    cout<<"GRESKA!\nPri pokretanju programa morate unijeti tocno 2 argumenta: N i M!"<<endl;
    exit(EXIT_FAILURE);
  }
  
  srand(time(0));
  int N = atoi(argv[1]) <= 10 ? atoi(argv[1]) : 10, M = atoi(argv[2]);
  cout<<"Main - N: "<<N<<" | M: "<<M<<endl;
  int arg_polje[] = {N, M};

  polje_brojeva = new int[N];

  pthread_t id_prvi, id_drugi;

  pthread_create(&id_prvi, NULL, generiranje, arg_polje);
  pthread_create(&id_drugi, NULL, racunanje, arg_polje);

  pthread_join(id_prvi, NULL);  
  pthread_join(id_drugi, NULL);
  
  delete[] polje_brojeva;
  return 0;
};