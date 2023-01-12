#include <stdio.h>

int main(void) {
    float salary;

    printf("\a원하는 월급 액수를 입력:");
    printf("$____\b\b\b\b");
    scanf_s("%f", &salary);
    printf("\n\t월 $%.2f이면 연봉은 $%.2f이다", salary, salary * 12);
    printf("\r계산하면, \n");


    return 0;
}