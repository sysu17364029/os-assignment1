#include <semaphore.h>

int main() {
    sem_unlink("/INIT");
    sem_unlink("/FULL");
    sem_unlink("/EMPTY");
    return 0;
}
