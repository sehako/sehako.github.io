#include <stdio.h>

int main(void) {
    int num = 10;
    int * ptr;
    int val;

    ptr = &num;
    val = *ptr;
    printf("val = %d", val);
    
    return 0;
}