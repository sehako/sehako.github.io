#include <stdio.h>
#include <stdlib.h>

int main(void) {
    double * ptd;
    int max;
    int number;
    int i = 0;

    puts("몇 개의 double을 입력할건지?");

    if (scanf_s("%d", &max) != 1) {
        puts("숫자가 정확하게 입력되지 않았습니다.");
        exit(EXIT_FAILURE);
    }
    ptd = (double *) malloc(max * sizeof(double));

    if (ptd == NULL) {
        puts("메모리 할당에 실패했다.");
        exit(EXIT_FAILURE);
    }

    puts("값들을 입력하시오(q입력시 종료)");

    while (i < max && scanf_s("%lf", &ptd[i]) == 1) {
        ++i;
    }
    printf("입력한 %d개의 값들은 다음과 같다\n", number = i);

    for (i = 0; i < number; i++) {
        printf("%7.2f", ptd[i]);
        if(i % 7 == 6) {
            putchar('\n');
        }
    }
    if(i % 7 != 0) {
        putchar('\n');
    }
    puts("종료");
    free(ptd);

    return 0;
}