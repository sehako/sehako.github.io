#include <stdio.h>
#define SIZE 4

int main(void) {
    short array[SIZE];
    short * pti;
    short index;
    double array_d[SIZE];
    double * ptf;

    pti = array;
    ptf = array_d;
    printf("%23s %15s\n", "short", "double");
    for (index = 0; index < SIZE; index++) {
        printf("포인터 + %d: %10p %10p\n", index, pti + index, ptf + index);
    }

    return 0;
}