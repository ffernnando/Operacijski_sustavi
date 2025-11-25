#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>


using namespace std;

int N;
int trazim_id, broj_id;
int *TRAZIM, *BROJ;

void brisi(int sig) {
  shmdt(TRAZIM);
  shmctl(trazim_id, IPC_RMID, NULL);

  shmdt(BROJ);
  shmctl(broj_id, IPC_RMID, NULL);
  exit(0);
};

void sloziSegment() {
  trazim_id = shmget(IPC_PRIVATE, sizeof(int) * N, 0600);
  broj_id = shmget(IPC_PRIVATE, sizeof(int) * N, 0600);
  if (trazim_id == -1 || broj_id == -1) {
    exit(1);
  }

  TRAZIM = (int*)shmat(trazim_id, NULL, 0);
  BROJ = (int*)shmat(broj_id, NULL, 0);


  for(int i = 0; i < N; i++) {
    TRAZIM[i] = 0;
    BROJ[i] = 0;
  }
  
};




int maksimum() {
  int max = BROJ[0];
  for (int i = 1; i < N; i++) {
    if (BROJ[i] > max) max = BROJ[i];
  }
  return max;
};

void ulazKO(int i) {
  TRAZIM[i] = 1;
  BROJ[i] = maksimum() + 1;
  TRAZIM[i] = 0;

  for(int j = 0; j < N; j++) {
    while (TRAZIM[j] != 0) {
      sleep(1);
    }
    while (BROJ[j] != 0 && (BROJ[j] < BROJ[i] || ((BROJ[j] == BROJ[i]) && (j < i)))) {
      sleep(1);
    }
  }
};

void izlazKO(int i) {
  BROJ[i] = 0;
};

void proc(int i) {
  for (int k = 1; k <= 5; k++) {
    ulazKO(i);
    for (int m = 1; m <= 5; m++) {
      cout<<"Proces: "<<i+1<<", K.O. br: "<<k<<" ("<<m<<"/5)"<<endl;
    }
    izlazKO(i);
  }
};

int main(int argc, char* argv[]) {
  sigset(SIGINT, brisi);

  N = atoi(argv[1]);
  sloziSegment();

  int i;
  for(i = 0; i < N; i++) {
    switch(fork()) {
      case 0: {
        proc(i);
        exit(0);
      }
      case -1: {
        cout<<"GRESKA!"<<endl;
        break;
      }
    }
  }

  for (int j = 0; j < N; j++) {
    wait(NULL);
  }
  brisi(0);
  return 0; 
};