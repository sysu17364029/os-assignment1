#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/***********************************************************
*    The implementation of the example in the textbook is  *
*  based on the thought that neighbors never eat simulta-  *
*  neously. Actually, the example uses monitor to implem-  *
*  ent exclusive access, and semaphore to send signal. To  *
*  implement a similar solution with POSIX mutex locks as  *
*  well as condition variables, we can simply use a mutex  *
*  lock to promise exclusion, and a condition variable to  *
*  send signals.                                           *
************************************************************/


#define num 5
#define LEFT (id+4)%5
#define RIGHT (id+1)%5

enum{THINKING, HUNGRY, EATING} state[num];  // State of the phils
pthread_mutex_t mutex; 
pthread_cond_t mutex_f[num];  // The 'can eat' signal


/*
 *  Check if phil %id% can eat instantly. If so, set its state and send a signal.
 */
void check(int id) {
    if (state[id] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[id] = EATING;
	pthread_cond_signal(&mutex_f[id]);
    }
}

/*
 *  The function implements the following process: 
 *    1. Be hungry.
 *    2. Wait until able to eat.
 *    3. Pick up the forks and starts to eat for a random period.
 *    4. Return the length of eating.
 */
int pickup_forks(int id) {
    pthread_mutex_lock(&mutex);  // Enter critical part
    state[id] = HUNGRY;  // Set the state to hungry
    printf("Philosopher %d is hungry!\n", id+1);
    check(id);  // Check if able to eat instantly
    if (state[id] != EATING) {
	pthread_cond_wait(&mutex_f[id], &mutex);  // Wait until can eat
    }
    unsigned t = 1000 + rand() % 2000;  // Generate a eating length
    printf("Philosopher %d will be eating for %.3lf seconds!\n", id+1, t / 1000.0);
    pthread_mutex_unlock(&mutex);  // Exit critical part
    return t;
}
/*
 *  The function implements the following process:
 *    1. Return the forks and try to send 'can-eat' signal to neighbors.
 *    2. Starts to think a random period.
 *    3. Return the length of thinking.
 */
int return_forks(int id) {
    pthread_mutex_lock(&mutex);  // Enter critical part
    state[id] = THINKING;  // Set the state to thinking
    check(LEFT);  // Check if %LEFT% can eat
    check(RIGHT);  // Check if %RIGHT% can eat
    unsigned t = 1000 + rand() % 2000;  // Generate a thinking length
    printf("Philosopher %d is thinking for %.3lf seconds!\n", id+1, t / 1000.0);
    pthread_mutex_unlock(&mutex);  // Exit critical part
    return t;
}

/* 
 * The philosopher function.
 */
void *philosopher(void *args) {
    int id = * (int*)args;
    int t = 1000 + rand() % 2000;  // Thinking time
    // Start to think at first
    printf("Philosopher %d is thinking for %.3lf seconds!\n", id+1, t / 1000.0);
    for (int i = 0; i < 5; ++i) {  // Think-and-eat for 5 turns
        usleep(1000*t);  // Thinking
	pickup_forks(id);  // Hungry and eat
	usleep(1000*t);  // Eating
	return_forks(id);  // Return forks
    }
}

int main() {
    srand((int)time(0));
    int ids[num];
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < num; ++i) {
        pthread_cond_init(mutex_f + i, NULL);
	state[i] = THINKING;
    }
    pthread_t pthreads[num];
    for (int i = 0; i < num; ++i) {
	ids[i] = i;
        pthread_create(pthreads + i, NULL, philosopher, (void*)(ids + i));
    }
    for (int i = 0; i < num; ++i) {
        pthread_join(pthreads[i], NULL);
    }
    return 0;
}
