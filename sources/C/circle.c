#include <stdio.h>
#define PI 3.14159

int main(void) {
    float radius;
    const float two = 2;

    printf("원의 반지름 입력:");
    scanf_s("%f", &radius);

    printf("원의 둘레 = %f\n원의 넓이 = %f", two * radius * PI, radius * radius * PI);
    return 0;
}