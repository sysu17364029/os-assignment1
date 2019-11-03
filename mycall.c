#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
	long a = syscall(333);
	printf("Syscall returns %d\n", a);
	return 0;
}

/*
int main() {
	int i = 0;
	int j = 0;
	for(i = 0; i <= 1000000000; i++)
		j++;
	long a = syscall(333);
	printf("Syscall returns %d\n", a);
	return 0;
}
*/

/*
int main() {
	int i = 0;
	for (i = 0; i <= 10000; i++)
		usleep(1);
	long a = syscall(333);
	printf("Syscall returns %d\n", a);
	return 0;
}
*/