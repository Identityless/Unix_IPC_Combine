#include <stdio.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#define MEMSIZE 4;

void provide(void *memory_segment, int clientnum);
void* componentsFactory();

int components = 0;
static sem_t sync_sem;
char signal[10] = {0}; 

typedef struct product {
    int components;
    struct timespec tp;
} Product;


void main() {
    int shmid[10];
    int initvalue = -1;
    int clientnum=0;
    int memsize = MEMSIZE;
    key_t keys[] = {(key_t)60110, (key_t)60111, (key_t)60112, (key_t)60113, (key_t)60114, (key_t)60115, (key_t)60116, (key_t)60117, (key_t)60118, (key_t)60119}; // 이거 외 상수로 하믄 않됨?
    pthread_t threads[3];
    int rc;
    void *memory_segment=NULL;
    Product initstate;
    initstate.components = initvalue;
    sem_init(&sync_sem, 0, 1);

    for(int i = 0 ; i < 10 ; i++) {
        if((shmid[i]=shmget(keys[i],sizeof(Product),IPC_CREAT|0666))==-1) {
            printf("shmget faild. (point 1)\n");
        }
        if((memory_segment=shmat(shmid[i],NULL,0))==(void*)-1) {    // shared moemory 포인터를 바꿔가면서 검사
            printf("shmat failed. (point 2)\n");
            exit(0);
        }
        memcpy((Product*)memory_segment, &initvalue, memsize);
    }

    for(int i = 0 ; i < 3 ; i++) {
        printf("In main : creating thread %d\n", i);
        rc = pthread_create(&threads[i], NULL, componentsFactory, NULL);
        if(rc) {
            printf("[ERROR] : return code from pthread_create is %d\n", rc);
            exit(-1);
        }
    }

    while(1) {
        if((memory_segment=shmat(shmid[clientnum],NULL,0))==(void*)-1) {    // shared moemory 포인터를 바꿔가면서 검사
            printf("shmat failed. (point 3, clientnum : %d)\n", clientnum);
            exit(0);
        }
        provide(memory_segment, clientnum);
        clientnum = ++clientnum % 10;
    }

}

void provide(void *memory_segment, int clientnum) {
    Product product;
    product.components = components;
    printf("Rotate num : %d\n", clientnum);
    Product* buffer = (Product*)memory_segment;
    int bufferquantity = buffer->components;
    int memsize = sizeof(Product);
    if(components > 0 && bufferquantity < 3 && bufferquantity != -1) {
        sem_wait(&sync_sem);
        components--;
        sem_post(&sync_sem);
        buffer->components++;
        clock_gettime(CLOCK_MONOTONIC, &buffer->tp);
        memcpy((Product*)memory_segment, buffer, memsize);
    }
    usleep(500000);
}

void* componentsFactory() {
    srand(time(NULL));
    int productivity = rand()%4+5;

    while(1){
        if(components < 20) {
            sem_wait(&sync_sem);
            components++;
            sem_post(&sync_sem);
            printf("Thread : Component Supplied, (Quentity : %d)\n", components);
        }

        sleep(productivity);
    }
}