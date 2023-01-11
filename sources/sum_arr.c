#include <stdio.h>
#define SIZE 10

int sum(int ar[], int n);

int main(void) {
    int array[SIZE] = {20, 14, 26, 34, 25, 45, 11, 53, 16, 32};
    long answer;

    answer = sum(array, SIZE);
    printf("총 합 = %ld\n",answer);
    printf("array의 크기는 %zd바이트\n", sizeof array);

    return 0;
}

int sum(int ar[], int n) {
        int i;
    int total = 0;

    for (i = 0; i < n; i++) {
        total += ar[i];
    }

    printf("ar의 크기는 %zd바이트\n", sizeof ar);
    
    return total;
}