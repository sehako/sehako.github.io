#include <stdio.h>
#define DEF "This is #define"

int main(void) {
    char str[80] = "This is String";
    const char * strg = "This is pointer String";

    puts(DEF);
    puts(str);
    puts(strg);
    puts(&str[5]);
    puts(strg + 4);

    return 0;
}