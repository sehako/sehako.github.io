// 하나의 파일에서 두 개의 함수를 사용

#include <stdio.h>
void hello(void);   // ANSI/ISO C 함수 프로토타입

int main(void) {
    printf("인사를 하는 함수를 구현해보자\n");
    hello();

    return 0;
}

void hello() {
    printf("안녕하세요!\n");
}