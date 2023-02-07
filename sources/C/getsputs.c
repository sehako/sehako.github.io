#include <stdio.h>
#define STLEN 81

int main(void) {
    char words[STLEN];

    puts("Input String: ");
    gets(words);
    printf("-----------\n");
    printf("%s\n", words);
    puts(words);
    puts("END.");

    return 0;
}