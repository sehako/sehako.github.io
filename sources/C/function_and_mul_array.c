#include <stdio.h>
#define ROWS 3
#define COLS 4

void sum_rows(int ar[][COLS], int rows);
void sum_cols(int [][COLS], int );  //이름을 생략하는 것이 가능하다
int sum2d(int (* ar)[COLS], int);   //또 다른 프로토타입 신택스

int main(void) {
    int arr[ROWS][COLS] = {
        {2, 4, 6, 8},
        {3, 5, 7, 9},
        {12, 10, 8, 6}
    };

    sum_rows(arr, ROWS);
    sum_cols(arr, ROWS);
    printf("모든 원소들의 합계: %d\n", sum2d(arr, ROWS));

    return 0;
}

void sum_rows(int ar[][COLS], int rows) {
    int r, c, tot;

    for (r = 0; r < rows; r++) {
        tot = 0;
        for (c = 0; c < COLS; c++) {
            tot += ar[r][c];
        }
        printf("%d행 합: %d\n", r + 1, tot);
    }
}

void sum_cols(int ar[][COLS], int rows) {
    int r, c, tot;

    for (c = 0; c < COLS; c++) {
        tot = 0;
        for (r = 0; r < rows; r++) {
            tot += ar[r][c];
        }
        printf("%d열 합: %d\n", c + 1, tot);
    }
}

int sum2d(int ar[][COLS], int rows) {
    int r, c, tot;

    tot = 0;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < COLS; c++) {
            tot += ar[r][c];
        }
    }

    return tot;
}