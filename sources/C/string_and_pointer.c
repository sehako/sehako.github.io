#define MSG "I'm special."

#include <stdio.h>

int main(void) {
    char ar[] = MSG;
    const char *pt = MSG;
    
    printf("I'm special.의 주소값: %p\n", "I'm special.");
    printf("ar: %p\n", ar);
    printf("pt: %p\n", pt);
    printf("MSG: %p\n", MSG);
    printf("I'm special.의 주소값: %p\n", "I'm special.");

    return 0;
}