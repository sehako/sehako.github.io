#include <stdio.h>
#define TEN 10
int main(void) {
    int ten[TEN] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int index;

    for(index = 0; index < TEN; index++) {
        printf("%d\n", *(ten + index)); //ten[index]와 같다
    }

    return 0;
}