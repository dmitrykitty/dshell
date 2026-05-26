#include <stdio.h>
#include <unistd.h>

int main(void) {
    int counter = 0;

    printf("loop started, pid = %d\n", getpid());
    fflush(stdout);

    while (1) {
        sleep(1);
    }

    return 0;
}