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
#define NUM_CONS 3

pthread_t pthreads[NUM_CONS];

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

void* consumer(void *lamda_c) {
    double c = *(double*)lamda_c;
    while(1) {
	usleep(1000.0*Wait(c));
	pthread_mutex_lock(&lock);  // exlusive among threads
        sem_wait(empty);  // wait until not empty
	int d = q->data[q->head];
	printf("Consume: pid = %d, tid = %d, data = %d\n", getpid(), syscall(SYS_gettid), d);
	q->head = (q->head+1) % SIZE;
	sem_post(full);  // allow producer to produce
	pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *args[]) {
    if (argc != 2) {
        printf("Invalid argument number! Please call like './cons 50'!\n");
	return 0;
    }
    double c = atof(args[1]);
    int shmid;
    shmid = shmget(SHM_ID, sizeof(struct queue), 0660 | IPC_CREAT);
    if (shmid == -1) {
        printf("Error when opening or creating shared memory!\n");
	return -1;
    }
    void *ptr = shmat(shmid, NULL, !SHM_RDONLY);
    if (ptr == NULL) {
        printf("Error when linking to shared memory!\n");
	return -1;
    }
    q = (struct queue*)ptr;
// Exclusively initialize the shared-memory queue
    init = sem_open("/INIT", O_CREAT | O_EXCL, 0666, 0);  // Try to create shared semaphore
    if(init == SEM_FAILED) {  // If failed, wait until initalization ends
	printf("Here1.\n");
	init = sem_open("/INIT", 0);
	if (init == SEM_FAILED) perror(strerror(errno));
	sem_wait(init);
	printf("Waited.\n");
    }
    else {  // If succeed, initialize the queue
        printf("Here2.\n");
	q->head = 0;
	q->tail = 0;
	memset(q->data, 0, sizeof(q->data));
	sem_post(init);
    }
// Initialize semaphores related to queue
    full = sem_open("/FULL", O_CREAT, 0666, SIZE);
    empty = sem_open("/EMPTY", O_CREAT, 0666, 0);
    srand(time(NULL));
    for (int i = 0; i < NUM_CONS; ++i) {
        pthread_create(pthreads+i, NULL, consumer, (void*)&c);
    }
    for (int i = 0; i < NUM_CONS; ++i) {
        pthread_join(pthreads[i], NULL);
    }
    return 0;
}
