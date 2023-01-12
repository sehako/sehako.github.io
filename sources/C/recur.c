#include <stdio.h>

void rec(int);

int main(void) {
    rec(1);
    return 0;
}

void rec(int n) {
    printf("Level %d: n의 메모리 주소 %p\n", n, &n);
    
    if(n < 4) {
        rec(n + 1);
    }
    printf("Level %d: n의 메모리 주소 %p\n", n, &n);

}