#include <stdio.h>

int main(void) {
    int arr[2][2] = {
        {4, 2}, 
        {3, 7}
        };

    printf("arr = %p,   arr + 1 = %p\n", arr, arr + 1);
    printf("arr[0] = %p,   arr[0] + 1 = %p\n", arr[0], arr[0] + 1);
    printf("*arr = %p,   *arr + 1 = %p\n", *arr, *arr + 1);
    printf("arr[0][0] = %d\n", arr[0][0]);
    printf("*arr[0] = %d\n", *arr[0]);
    printf("**arr = %d\n", **arr);
    printf("arr[1][0] = %d\n", arr[1][0]);
    printf("*(*(arr + 2) + 1) = %d\n", *(*(arr + 1) + 1));
    
    return 0;
}