#include <stdio.h>
#define SLEN 20
#define LIM 5

int main(void) {
    const char *arr[LIM] = {
        "Hello",
        "World!",
        "This",
        "is",
        "C language"
    };

    char ar[LIM][SLEN] = {
        "String",
        "and",
        "Pointer",
        "very",
        "interesting!"
    };

    for(int i = 0; i < LIM; i++) {
        printf("%-36s%-25s\n", arr[i], ar[i]);
    }
    printf("\nsizeof arr: %zd, sizeof ar: %zd\n", sizeof(arr), sizeof(ar));

    return 0;
}