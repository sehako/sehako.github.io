#include <stdio.h>

int main(void) {
    float height;
    float value;

    printf("키 입력(cm 단위)");
    scanf_s("%f", &height);

    value = height * 10;
    printf("당신의 키를 mm단위로 환산하면 %f이다.", value);

    return 0;
}