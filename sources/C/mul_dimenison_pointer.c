#include <stdio.h>

int main(void) {
    int arr[2][2] = {
        {3, 5},
        {7, 4}
    };
    int (* ptr)[2];
    ptr = arr;

    printf("ptr = %p, ptr + 1 = %p\n", ptr, ptr + 1);
    printf("ptr[0] = %p, ptr[0] + 1 = %p\n", ptr[0], ptr[0] + 1); 
    printf("*ptr = %p, *ptr + 1 = %p\n", *ptr, *ptr + 1);
    printf("ptr[0][0] = %d\n", ptr[0][0]);
    printf("*ptr[0] = %d\n", *ptr[0]);
    printf("**ptr = %d\n", **ptr);
    printf("ptr[1][0] = %d\n", ptr[1][0]);
    printf("*(*(ptr + 1) + 1) = %d\n", *(*(ptr + 1) + 1));
    
    return 0;
}