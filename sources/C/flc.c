#include <stdio.h>
#define COLS 4

int sum(int ar[][COLS], int rows);
int total(int ar[], int n);

int main(void) {
    int total1, total2, total3;
    int * ptr;
    int (* pt)[COLS];

    ptr = (int [2]){10, 20};
    pt = (int [2][COLS]) {{1,2,3,4}, {4,5,6,7}};

    total1 = total(ptr, 2);
    total2 = sum(pt, 2);
    total3 = total((int []){1,2,3,4,5,6}, 6);

    printf("total1 = %d\ntotal2 = %d\ntotal3 = %d", total1, total2, total3);

    return 0;
}

int total(int ar[], int n) {
    int i;
    int total = 0;

    for (i = 0; i < n; i++) {
        total += ar[i];
    }

    return total;
}

int sum(int ar[][COLS], int rows) {
    int r;
    int c;
    int tot = 0;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < COLS; c++) {
            tot += ar[r][c];
        }
    }

    return tot;
}