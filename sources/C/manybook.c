#include <stdio.h>
#include <string.h>

#define MAXTITL 40
#define MAXAUTL 40
#define MAXBKS 50

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book library[MAXBKS];
    int count = 0;
    int index;

    printf("도서 제목 입력: ");

    while(count < MAXBKS && gets_s(library[count].title, MAXTITL) != NULL && library[count].title[0] != '\0') {
        printf("저자 이름 입력: ");
        gets_s(library[count].author, MAXAUTL);
        printf("정가 입력: ");
        scanf_s("%f", &library[count++].value);

        while(getchar() != '\n') {
            continue;
        }
        if(count < MAXBKS) {
            printf("다음 타이틀 입력(끝내려면 [enter] 입력): ");
        }
    }

    if(count > 0) {
        printf("소장하고 있는 도서들의 목록:\n");
        for(index = 0; index < count; index++) {
            printf("%s, %s, %f\n", library[index].title, library[index].author, library[index].value);
        }
    }
    else {
        printf("책이 없습니다.\n");
    }

    return 0;
}