#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctime>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

/* PODACI ZAJEDNIČKI ZA PROIZVOĐAČE I POTROŠAČE
   int ULAZ - index elementa koji proizvođač upisuje - sizeof(int)
   int IZLAZ - index elementa koji potrošač čita - sizeof(int)
   int UKUPNO - m (broj proizvođača) * n (broj brojeva po proizovđaču) - sizeof(int)
   buffer: int M[5] - pohrana generiranih brojeva - sizeof(int) * 5;
      dijeljena memorija: sizeof(int) * 8;
   imenovani semafori PIŠI - bsem[0], PUN - osem[1], PRAZAN - osem[2]
      PIŠI.v = 1, PUN.v = 5, PRAZAN.v = 0
*/

// ------------------------------------ PREDUVJET ZA DIJELJENU MEMORIJU I SEMAFORE ------------------------------------

struct DijeljeniPodaci {
   int ULAZ;
   int IZLAZ;
   int UKUPNO;
   int M[5];
};
struct DijeljeniPodaci *dijeljeniPodaci;
int shmid;

int SemId;
union semun { int val; struct semid_ds *buf; unsigned short *array; };


// ------------------------------------ FUNKCIJE ZA RAD S IMENOVANIM SEMAFORIMA ------------------------------------
int SemGet(int n) {
   SemId = semget(IPC_PRIVATE, n, 0600);

   return SemId;
};

int SemSetVal(int SemNum, int SemVal) {
   union semun arg;
   arg.val = SemVal;
   return semctl(SemId, SemNum, SETVAL, arg);
};

int SemOp(int SemNum, int SemOp) {
   struct sembuf SemBuf;

   SemBuf.sem_num = SemNum;
   SemBuf.sem_op = SemOp;
   SemBuf.sem_flg = 0;
   
   return semop(SemId, &SemBuf, 1);
};

void SemRemove() {
   semctl(SemId, 0, IPC_RMID, 0);
};

// ------------------------------------ DIO ZA PROIZVOĐAČA I POTROŠAČA  ------------------------------------

void proizvodjac(int n, int indeks) {
   srand(getpid());
   for (int i = 0; i < n; i++) {
      SemOp(1, -1); //ispitaj je li puno
      SemOp(0, -1); //ispitaj piše li netko

      dijeljeniPodaci->M[dijeljeniPodaci->ULAZ] = rand();
      cout << "Proizvodac " << indeks << ". salje \"" << dijeljeniPodaci->M[dijeljeniPodaci->ULAZ] << "\"" << endl; 

      dijeljeniPodaci->ULAZ = (dijeljeniPodaci->ULAZ + 1) % 5;

      SemOp(0, 1); //oslobodi pisanje
      SemOp(2, 1); //povećaj prazan za 1
  }
  cout << "Proizvodac " << indeks << " zavrsio sa slanjem" << endl;
};

void potrosac() {
  //potrosac čita broj po broj koji mu proizvođači šalju u spremnik
  //i onda ih dodaje zbroju
  long long zbroj = 0;
  for (int i = 0; i < dijeljeniPodaci->UKUPNO; i++) {
      SemOp(2, -1); //ispitaj prazan - umanji prazan za 1 - uklonilo se jedno neprazno mjesto
      cout << "Potrosac prima " << dijeljeniPodaci->M[dijeljeniPodaci->IZLAZ] << endl;
      zbroj += dijeljeniPodaci->M[dijeljeniPodaci->IZLAZ];
      dijeljeniPodaci->IZLAZ = (dijeljeniPodaci->IZLAZ + 1) % 5;
      
      SemOp(1, 1); //postavi pun - oslobodilo se jedno mjesto za upis u buffer
  }
  cout << "Potrosac - zbroj primljenih brojeva = " << zbroj << endl;
};

void brisi() {
   shmdt(dijeljeniPodaci);
   shmctl(shmid, IPC_RMID, NULL);
};

int main(int argc, char* argv[]) {
   if (argc != 3) {
      cout << "POGRESAN BROJ ULAZNIH PARAMETARA! MORATE UNIJETI BROJEVE m I n!" << endl;
      return 0;
   }

   //m - broj procesa proizvođača
   int m = atoi(argv[1]);
   //n - broj slučajnih brojeva koje svaki proizvođač generira
   int n = atoi(argv[2]);

   shmid = shmget(IPC_PRIVATE, sizeof(DijeljeniPodaci), 0600);
   if (shmid == -1) {
      cout << "KREIRANJE SEGMENTA DIJELJENE MEMORIJE NEUSPJELO!" << endl;
      exit(1);
   }   

   dijeljeniPodaci = (DijeljeniPodaci*) shmat(shmid, NULL, 0);
   
   //početne vrijednosti za dijeljene podatke
   dijeljeniPodaci->ULAZ = 0;
   dijeljeniPodaci->IZLAZ = 0;
   dijeljeniPodaci->UKUPNO = m*n;

   //napravi skup od 3 semafora - PIŠI, PUN, PRAZAN i postavi im početne vrijednosti
   if (SemGet(3) == -1) {
      brisi();
      //SemRemove();
      exit(1);
   }
   SemSetVal(0, 1);
   SemSetVal(1, 5);
   SemSetVal(2, 0);

   switch(fork()) {
      case -1:
         cout << "GRESKA PRI KREIRANJU NOVOG PROCESA POTROSACA" << endl;
         brisi();
         SemRemove();
         exit(1);
         break;
      case 0:
         potrosac();
         exit(0);
         break;
      default:
         for (int i = 1; i <= m; i++) {
            switch(fork()) {
               case -1: 
                  cout << "GRESKA PRI KREIRANJU NOVOG PROCESA PROIZVODJACA!" << endl;
                  brisi();
                  SemRemove();
                  exit(1);
                  break;
               case 0: 
                  proizvodjac(n, i);
                  exit(0);
                  break;
            }
         }
   }

   //pričekaj m procesa proizvođača + 1 proces potrošač ! !!!
   for (int i = 0; i < (m+1); i++) {
      wait(NULL);
   }

   //OBAVEZNO na kraju izbriši semafore i segment dijeljene memorije
   brisi();
   SemRemove();
   return 0;
};

/*
proces proizvođač
   i = 0
   čini
      //Ako buffer nije popunjen
      ispitaj_PUN

      //I ako nitko trenutačno ne piše u buffer
      ispitaj_PIŠI

      //postavi slučajni broj u buffer M na index ULAZ (default je 0)
      M[ULAZ] = slučajni broj

      //ciklički povećavaj ULAZ za 1
      ULAZ = (ULAZ + 1) mod 5

      //signaliziraj da je pisanje završilo
      postavi_PIŠI


      postavi_PRAZAN

      //povećaj i - preko i gledaš je li izgenerirano n brojeva
      i = i + 1
   do N // broj slučajnih brojeva
kraj.


proces potrošač
   //brojač i inicijaliziraj na 0 - preko njega gledaš jesi li sve brojeve
   i = 0

   //inicijaliziraj sumu na 0 - povećava se za vrijednost pročitanog broja u svakoj iteraciji petlje
   zbroj = 0

   //petlja koja ide do i < (m*n)
   čini
      //zapravo bi bilo bolje nazvati ispitaj_NEPRAZAN jer će se ostatak koda izvršavati
      //tek kad je semafor PRAZAN postavljen: PRAZAN.v > 0 ? dalje : čekaj
      ispitaj_PRAZAN

      //zbroju se dodaje vrijednost iz buffera na indexu izlaz koji se ciklički povećava za 1
      //to je u biti kursor za čitanje potrošača
      zbroj += M[IZLAZ]

      //ciklički povećaj izlaz za 1
      IZLAZ = (IZLAZ+1) mod 5

      //postavi semafor PUN, odnosno omogući da neki proizvođač prepiše novu vrijednost
      //preko postojeće jer je ona iskorištena
      postavi_PUN

      //uvećaj brojač
      i = i+1
   do UKUPNOg broja slučajnih brojeva

   //cout << zbroj << endl;
   ispiši zbroj na zaslon
kraj.
*/