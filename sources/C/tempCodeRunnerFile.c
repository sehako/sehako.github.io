#include <stdio.h>
#define SIZE 10

int sump(int * start, int * end);

int main(void) {
    int array[SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int answer;

    answer = sump(array, array + SIZE);

    printf("합 = %d\n", answer);

    return 0;
}

int sump(int * start, int * end) {
    int total = 0;

    while (start < end) {
        total += *start;
        start++;
    }

    return total;
}