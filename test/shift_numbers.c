#include <stdio.h>

/*
 * Test helper for DShell redirection and pipes.
 * Reads integers from stdin and prints each value plus one.
 */
int main(void) {
    int value;

    while (scanf("%d", &value) == 1) {
        printf("%d\n", value + 1);
    }

    return 0;
}
