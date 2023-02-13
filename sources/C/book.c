#include <stdio.h>
#include <string.h>
#define MAXTITL 41
#define MAXAUTL 31

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book library;    //library를 book형 변수로 선언

    printf("도서 제목 입력: ");
    gets_s(library.title, MAXTITL);
    printf("저자명 입력: ");
    gets_s(library.author, MAXAUTL);
    printf("정가 입력: ");
    scanf_s("%f", &library.value);
    printf("%s, %s, %f", library.title, library.author, library.value);
    printf("\n종료");

    return 0;
}