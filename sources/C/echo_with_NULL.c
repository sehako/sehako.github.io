#include <stdio.h>
#define STLEN 10

int main(void) {
    char words[STLEN];
    
    puts("Input String: ");
    while (fgets(words, STLEN, stdin) != NULL && words[0] != '\n') {
        fputs(words, stdout);
    }
    puts("END.");

    return 0;
}