#include <iostream>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

using namespace std;

struct shmStruct {
    int ulaz;
    int izlaz;
    int ukupno;
    int M[5];
};


int shmId, semId, N, B, PISI, PUN, PRAZAN;
shmStruct *shmData;

void dealocate() {
    shmdt(shmData);
    shmctl(shmId, IPC_RMID, NULL);
    semctl(semId, 0, IPC_RMID, 0);
    exit(0);
}

void SemInit() {
    semId = semget(IPC_PRIVATE, 3, 0600);
    if(semId == -1) {
        cout << "\n--- Greska kod alokacije semafora!\n";
        dealocate();
        exit(1);
    }

    PISI=0;
    PUN=1;
    PRAZAN=2;
    
    semctl(semId, PISI, SETVAL, 1); 
    semctl(semId, PUN, SETVAL, 5);
    semctl(semId, PRAZAN, SETVAL, 0);
}

void SemWait(int i) {
    struct sembuf s;
    s.sem_num = i;
    s.sem_op = -1;
    s.sem_flg = 0;
    semop(semId, &s, 1);
}

void SemPost(int i) {
    struct sembuf s;
    s.sem_num = i;
    s.sem_op = 1;
    s.sem_flg = 0;
    semop(semId, &s, 1);
}



void CreateProces(int index) {
    
    srand(getpid());

    cout << "--- PROIZVODJAC br. " << index << ", zapocinje s radom!\n";
    int br;

    for (int i = 0; i < B; i++) {
        SemWait(PUN);
        SemWait(PISI);

        br=rand()%1000+1;
        shmData->M[shmData->ulaz] = br;
        shmData->ulaz=(shmData->ulaz+1) % 5;

        cout << "--- PROIZVODJAC br. " << index << ", salje broj: " << br << endl;

        SemPost(PISI);
        SemPost(PRAZAN);
    }
    cout << "--- PROIZVODJAC br. "<< index <<", je zavrsio s radom!\n";
}


void ConsumeProces() {
    int num, sum = 0;
    cout << "> POTROSAC zapocinje s radom!\n";

    for (int i = 0; i < shmData->ukupno; i++) {
        SemWait(PRAZAN);
        num=shmData->M[shmData->izlaz];
        sum+=num;
        shmData->izlaz=(shmData->izlaz+1) % 5;
        SemPost(PUN);
        cout << "> POTROSAC, primio broj: " << num << endl;
    }

    cout << "> POTROSAC, zbroj: " << sum << endl;
}



int main(int argc, char* argv[]) {

    if(argc <= 2) {
        cout << "----- Nisu upisani argumenti! PREKID PROGRAMA...\n";
        exit(1);
    } else {
        N = atoi(argv[1]);
        B = atoi(argv[2]);
        if(N < 1 || N > 100 || B < 1 || B > 100) {
            cout << "----- Upisani argumenti nisu unutar raspona(1-100)! PREKID PROGRAMA...\n";
            exit(1);
        }
    }

    shmId = shmget(IPC_PRIVATE, sizeof(shmStruct), 0600);

    if(shmId == -1) {
        cout << "\n--- Greska kod alokacije dijeljene memorije (podaci)!\n";
        dealocate();
        exit(1);
    }

    shmData = (shmStruct*)shmat(shmId, NULL, 0);

    shmData->ukupno = N*B;

    SemInit();

    for(int i = 1; i<=N; i++) {   
        if (!fork()) {
            CreateProces(i);
            exit(0);
        }
    }

    if(!fork()) {
        ConsumeProces();
        exit(0);
    }

    for(int i = 0; i<=N; i++)
        wait(NULL);

    dealocate();

    return 0;
}