#include <stdio.h>

int main(void) {
    const char * msg = "This is msg";
    const char * copy;

    copy = msg;

    printf("%s\n", copy);
    printf("msg = %s    &msg = %p   값 = %p\n", msg, &msg, msg);
    printf("copy = %s   &copy = %p  값 = %p\n", copy, &copy, copy);

    return 0;
}