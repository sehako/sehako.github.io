#include <stdio.h>
#include <stdlib.h>

#define MAXTITL 40
#define MAXAUTL 40
#define MAXBKS 10

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book library[MAXBKS];
    int count = 0;
    int index, filecount;
    FILE * pbooks;
    int size = sizeof (struct book);

    if((pbooks = fopen("book.dat", "a + b")) == NULL) {
        fputs("book.bat 파일을 열 수 없음\n", stderr);
        exit(1);
    }
    
    rewind(pbooks); //파일의 시작으로 이동

    while(count < MAXBKS && fread(&library[count], size, 1, pbooks) == 1) {
        if(count == 0) {
            puts("book.dat에 들어 있는 내용: ");
        }
        printf("%s by %s: %f\n", library[count].title, library[count].author, library[count].value);
        count++;
    }
    filecount = count;

    if(count == MAXBKS) {
        fputs("book.dat 파일이 가득 차 있음\n", stderr);
        exit(2);
    }

    puts("새 도서 제목을 입력: ");
    puts("끝내려면 [enter]");

    while(count < MAXBKS && gets(library[count].title) != NULL && library[count].title[0] != '\0') {
        puts("저자명 입력: ");
        gets_s(library[count].author, MAXAUTL);
        puts("정가 입력: ");
        scanf_s("%f", &library[count++].value);

        while(getchar() != '\n') {
            continue;   //입력 라인을 깨끗이 비움
        }

        if(count < MAXBKS) {
            puts("다음 도서 제목 입력: ");
        }
    }

    if (count > 0) {
        puts("다음은 소장하고 있는 도서 목록이다:");

        for(index = 0; index < count; index++) {
            printf("%s by %s: %f\n", library[index].title, library[index].author, library[index].value);
        }
        fwrite(&library[filecount], size, count - filecount, pbooks);
    }
    else {
        puts("책이 한 권도 없음!");
    }

    puts("끝");
    fclose(pbooks);
}