#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 30

int main(int argc, char *argv[]) {
    FILE *in, *out;
    int ch;
    char name[LEN];
    int count = 0;

    if (argc < 2) {
        fprintf(stderr, "사용법: %s filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if((in = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "\"%s\" 파일을 열 수 없음\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    strncpy(name, argv[1], LEN - 5);
    name[LEN - 5] = '\0';
    strcat(name, ".red");

    if((out = fopen(name, "w")) == NULL) {
        fprintf(stderr, "출력 파일을 만들 수 없음\n");
        exit(3);
    }

    while((ch = getc(in)) != EOF) {
        if(count++ % 3 == 0) {
            putc(ch, out);
        }
    }

    if(fclose(in) != 0 || fclose(out) != 0) {
        fprintf(stderr, "파일을 닫는데 에러 발생\n");
    }

    return 0;
}