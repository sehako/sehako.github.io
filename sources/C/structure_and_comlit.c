#include <stdio.h>

#define MAXTITL 41
#define MAXAUTL 31

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book readfirst;
    int score;

    printf("테스트 스코어 입력: ");
    scanf_s("%d", &score);

    if(score >= 84) {
        readfirst = (struct book) {
            "How to C",
            "James",
            50000
            };
    }
    else {
        readfirst = (struct book) {
            "How to Python",
            "Hallow",
            30000
        };
    }

    printf("할당된 독서:\n");
    printf("%s, %s, %f", readfirst.title, readfirst.author, readfirst.value);

    return 0;
}