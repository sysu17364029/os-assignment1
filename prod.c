#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>

#define SIZE 20
#define SHM_ID 1234
#define NUM_PROD 3

pthread_t pthreads[NUM_PROD];

struct queue {
    int head;
    int tail;
    int data[SIZE];
} *q;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
sem_t *init, *full, *empty;

double Wait(double p) {
    double z;
    do {z = (double)rand()/RAND_MAX;} while ((z==0)||(z==1));
    return (-p*log(z));
}

void* producer(void *lamda_p) {
    double p = *(double*)lamda_p;
    while(1) {
	double st = Wait(p);
	usleep(1000.0*Wait(p));
	pthread_mutex_lock(&lock);  // exlusive among threads
        sem_wait(full);  // wait until not full
	int d = rand();
	printf("Produce: pid = %d, tid = %d, data = %d\n", getpid(), syscall(SYS_gettid), d);
	q->data[q->tail] = d;
	q->tail = (q->tail+1) % SIZE;
	sem_post(empty);  // allow consumer to use
	pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *args[]) {
    if (argc != 2) {
        printf("Invalid argument number! Please call like './prod 50'!\n");
	return 0;
    }
    double p = atof(args[1]);
// Init a shared-memory and test if it is a success.
    int shmid;
    shmid = shmget(SHM_ID, sizeof(struct queue), 0660 | IPC_CREAT);
    if (shmid == -1) {
        perror(strerror(errno));
	printf("Error when opening or creating shared memory!\n");
	return -1;
    }
// Link the shared-memory to local pointer and test if it is a success.
    void *ptr = shmat(shmid, NULL, !SHM_RDONLY);
    if (ptr == NULL) {
        printf("Error when linking to shared memory!\n");
	return -1;
    }
    q = (struct queue*)ptr;
// Exclusively initialize the shared-memory queue
    init = sem_open("/INIT", O_CREAT | O_EXCL, 0666, 0);  // Try to create shared semaphore
    if(init == SEM_FAILED) { 
 // If failed, wait until initalization ends
	init = sem_open("/INIT", 0);
	sem_wait(init);
    }
    else {  // If succeed, initialize the queue
	q->head = 0;
	q->tail = 0;
	memset(q->data, 0, sizeof(q->data));
	sem_post(init);
    }
    // Initialize semaphores related to queue
    full = sem_open("/FULL", O_CREAT, 0666, SIZE);
    if (full == SEM_FAILED) perror(strerror(errno));
    empty = sem_open("/EMPTY", O_CREAT, 0666, 0);
    if (full == SEM_FAILED) perror(strerror(errno));
    srand(time(NULL));
    for (int i = 0; i < NUM_PROD; ++i) {
        pthread_create(pthreads+i, NULL, producer, (void*)&p);
    }
    for (int i = 0; i < NUM_PROD; ++i) {
        pthread_join(pthreads[i], NULL);
    }
    return 0;
}
