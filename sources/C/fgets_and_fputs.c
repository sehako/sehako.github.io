#include <stdio.h>
#define STLEN 8

int main(void) {
    char words[STLEN];

    puts("Input String: ");
    fgets(words, STLEN, stdin);
    printf("------------------\n");
    puts(words);
    fputs(words, stdout);

    puts("Input String: ");
    fgets(words, STLEN, stdin);
    printf("------------------\n");
    puts(words);
    fputs(words, stdout);
    puts("END.");

    return 0;
}