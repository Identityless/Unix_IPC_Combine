#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>

#define MEMSIZE 4;

void checkUsableKey();
void choiceKey(key_t *key);
void init(void **mem_seg, key_t key);
void *makeCar(void *ptr);
void *paintCar(void *ptr);
void *inspectCar(void *ptr);
void getComponents();
void calDuration(struct timespec begin, struct timespec end);

static sem_t sync_sem;
int components = 0;

typedef struct Car
{
    bool isCreated;
    bool isPainted;
    bool isInspected;
    struct Car *next;
} Car;

typedef struct product {
    int components;
    struct timespec tp;
} Product;


void main()
{
    int msg;
    int memsize = MEMSIZE;
    int shmid;
    int initsetval = 0;
    key_t key;
    void *memory_segment = NULL;

    checkUsableKey();
    choiceKey(&key);
    //printf("point 1\n");

    if (sem_init(&sync_sem, 0, 1) == -1)
    {
        perror("sem_init");
        exit(1);
    }
    //printf("point 2\n");

    if ((shmid = shmget(key, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        printf("shmget faild\n");
    }
    //printf("point 3\n");

    if ((memory_segment = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        printf("shmat failed\n");
        exit(0);
    }
    //printf("point 4\n");

    memcpy((int *)memory_segment, &initsetval, sizeof(int));
    //printf("point 5\n");

    pthread_t thread[3];
    Car *headCar = (Car *)malloc(sizeof(Car));
    headCar->next = NULL;

    int err_code = pthread_create(&thread[0], NULL, makeCar, (void *)headCar); // 차를 만든다.
    if (err_code)
    {
        printf("ERROR code is %d\n", err_code);
        exit(1);
    }
    //printf("point 6\n");
    err_code = 0;
    err_code = pthread_create(&thread[1], NULL, paintCar, (void *)headCar); // 차를 도색한다.
    if (err_code)
    {
        printf("ERROR code is %d\n", err_code);
        exit(1);
    }
    //printf("point 7\n");
    err_code = 0;
    err_code = pthread_create(&thread[2], NULL, inspectCar, (void *)headCar); // 차를 검사한 후 출고한다.
    //printf("point 8\n");
    if (err_code)
    {
        printf("ERROR code is %d\n", err_code);
        exit(1);
    }
    //printf("point 9\n");
    
    Product* buffer;
    struct timespec ctp;
    //printf("point 10\n");
    while (1)
    {
        //clock_gettime(CLOCK_MONOTONIC, &ctp);
        //printf("check buffer : %d\n", buffer->components);
        //printf("check components : %d\n", components);

        if (components < 10 && ((Product*)memory_segment)->components > 0)
        {
            
            clock_gettime(CLOCK_MONOTONIC, &ctp);
            buffer = (Product*)memory_segment;
            buffer->components--;
            memcpy((Product*)memory_segment, buffer, sizeof(Product));
            
            printf("component get ! \n");
            sem_wait(&sync_sem);
            components++;
            sem_post(&sync_sem);
            
            calDuration(buffer->tp, ctp);
        }
        
    }

}

void calDuration(struct timespec begin, struct timespec end) {
     if (end.tv_nsec - begin.tv_nsec < 0){
        printf("Client : %ld.%09ld sec\n", end.tv_sec - begin.tv_sec - 1, end.tv_nsec - begin.tv_nsec + 1000000000);
    }
    else
    {
        printf("Client : %ld.%09ld sec\n", end.tv_sec - begin.tv_sec, end.tv_nsec - begin.tv_nsec);
    }
}

void choiceKey(key_t *key)
{
    int buffer = 0;

    printf("사용할 키 입력 : ");
    scanf("%d", &buffer);

    *key = (key_t)buffer;
}

void checkUsableKey()
{
    key_t keys[] = {(key_t)60110, (key_t)60111, (key_t)60112, (key_t)60113, (key_t)60114, (key_t)60115, (key_t)60116, (key_t)60117, (key_t)60118, (key_t)60119};
    int shmid[10];
    void *memory_segment = NULL;
    int readpoint;

    for (int i = 0; i < 10; i++)
    {
        if ((shmid[i] = shmget(keys[i], sizeof(int), IPC_CREAT | 0666)) == -1)
        {
            printf("shmget faild. (point 1)\n");
        }
        if ((memory_segment = shmat(shmid[i], NULL, 0)) == (void *)-1)
        { // shared moemory 포인터를 바꿔가면서 검사
            printf("shmat failed. (point 2)\n");
            exit(0);
        }
        memcpy(&readpoint, (int *)memory_segment, sizeof(int));

        if (readpoint == -1)
        {
            printf("%d key - 사용 가능\n", (int)keys[i]);
        }
        else
        {
            printf("%d key - 사용 불가\n", (int)keys[i]);
        }
    }
}

void *makeCar(void *ptr)
{
    int i = 0;
    Car *current_made_car = ptr;
    for (;;)
    {
        if (components > 0)
        {
            sem_wait(&sync_sem);
            components--; // 부품 갯수 빼주는 녀석
            sem_post(&sync_sem);

            //sleep(3);
            current_made_car->next = (Car *)malloc(sizeof(Car));
            current_made_car = current_made_car->next;
            current_made_car->isCreated = true;
            current_made_car->isPainted = false;
            current_made_car->isInspected = false;
            current_made_car->next = NULL;
            printf("car %d is created, client have %d components\n", i, components);
            i++;
        }
    }
}

void *paintCar(void *ptr)
{
    int i = 0;
    Car *current_painted_car = ptr;
    for (;;)
    {
        if ((current_painted_car->next != NULL) && current_painted_car->next->isCreated)
        {
            //sleep(1);
            current_painted_car = current_painted_car->next;
            current_painted_car->isPainted = true;
            printf("car %d is painted\n", i);
            i++;
        }
    }
}

void *inspectCar(void *ptr)
{
    int i = 0;
    Car *current_inspect_target = ptr;
    for (;;)
    {
        if (current_inspect_target->next != NULL && current_inspect_target->next->isPainted)
        {
            //sleep(2);
            Car *inpected_car = current_inspect_target;
            current_inspect_target = current_inspect_target->next;
            current_inspect_target->isInspected = true;
            free(inpected_car);
            printf("car %d is inspected\n", i);
            i++;
        }
    }
}
