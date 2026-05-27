#include <stdio.h>
#include <unistd.h>

/*
 * Small background-job helper.
 * It sleeps shortly so "jobs" can show it as running.
 */
int main(void) {
    printf("wait_and_print started pid=%d\n", getpid());
    fflush(stdout);
    sleep(10);
    printf("wait_and_print done\n");
    return 0;
}
