#include <iostream>
#include <cstdlib>

using namespace std;

void proizvodjac() {
  //proizvođači pune dijeljeni spremnik broj po broj
  //znači spremnik ima 5 mjesta max i onda ima kao buffer u kojem brojevi
  //čekaju dok ih potrošač ne pročita i doda zbroju
  //ULAZ = (ULAZ + 1) mod 5 => daje u biti da se za 5 indeksi
  //vraćaju na 0 pa 6 bude indeks 1 itd.
  //do while morti?
  for (int i = 0; i < n; i++) {

  }
};

void potrosac() {
  //potrosac čita broj po broj koji mu proizvođači šalju u spremnik
  //i onda ih dodaje zbroju

  int zbroj = 0;
  for (int i = 0; i < n; i++) {

  }
};


/*
proces proizvođač
   i = 0
   čini
      ispitaj_PUN
      ispitaj_PIŠI
      M[ULAZ] = slučajni broj
      ULAZ = (ULAZ + 1) mod 5
      postavi_PIŠI
      postavi_PRAZAN
      i = i + 1
   do N // broj slučajnih brojeva
kraj.


proces potrošač
   i = 0
   zbroj = 0
   čini
      ispitaj_PRAZAN
      zbroj += M[IZLAZ]
      IZLAZ = (IZLAZ+1) mod 5
      postavi_PUN
      i = i+1
   do UKUPNOg broja slučajnih brojeva
   ispiši zbroj na zaslon
kraj.
*/
void SemGet(int n) {  
   SemId = semget(IPC_PRIVATE, n, 0600);
   if (SemId == -1) {
      printf("Nema semafora!\n");
      exit(1);
   }
}

int main(int argc, char* argv[]) {
  /* int id = shmget(IPC_PRIVATE, sizeof(int) * N, 0600);
  if(id == -1) {
    return;
  }*/ 

  int semget()

  int br_pr = atoi(argv[1]);
  int br_br = atoi(argv[2]);


  return 0;
};