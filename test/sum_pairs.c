#include <stdio.h>

/*
 * Test helper for DShell pipelines.
 * Reads two integers at a time and prints their sum.
 */
int main(void) {
    int left;
    int right;

    while (scanf("%d%d", &left, &right) == 2) {
        printf("%d\n", left + right);
    }

    return 0;
}
