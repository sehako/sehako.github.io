#include <stdio.h>

void function(void);

int main(void) {
    for (int i = 0; i <= 3; i++) {
        printf("사이클: %d\n", i);
        function();
    }

    return 0;
}

void function(void) {
    int value = 1;
    static int var = 1;

    printf("%d  %d\n", value++, var++);
}