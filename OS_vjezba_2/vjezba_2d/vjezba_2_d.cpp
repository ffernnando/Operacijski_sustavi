#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>


using namespace std;

int Id;
int *ZajednickaVarijabla;

void Pisac(int i) {
  *ZajednickaVarijabla = i;
}

void Citac() {
  int i;

  do {
    i = *ZajednickaVarijabla;
    cout<<"Procitano "<<i<<endl;
    sleep(1);
  } while(i == 0);
  cout<<"Procitano je: "<<i<<endl;
}

void brisi(int sig) {
  shmdt(ZajednickaVarijabla);
  shmctl(Id, IPC_RMID, NULL);
  exit(0);
}

//dodati cleanup u slučaju interrutpta (ctr + c)
void izlaz(int sig) {
  cout<<"Baj baj!"<<endl;
  exit(0);
};



int main() {
  Id = shmget(IPC_PRIVATE, sizeof(int) * 100, 0600);
  if (Id == -1) exit(1);

  ZajednickaVarijabla = (int*) shmat(Id, NULL, 0);
  *ZajednickaVarijabla = 0;

  sigset(SIGINT, brisi);

  if (fork() == 0) {
    Citac();
    exit(0);
  }
  if (fork() == 0) {
    sleep(5);
    Pisac(123);
    exit(0);
  }
  wait(NULL);
  wait(NULL);
  brisi(0);
  //zauzimanje zajedničke memorije

  
  //oslobađanje zauzete zajedničke memorije
  return 0; 
};