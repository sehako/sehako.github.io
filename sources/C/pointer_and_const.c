#include <stdio.h>
#define SIZE 5

void show_array(const double ar[], int n);
void mult_array(double ar[], int n, double mult);

int main(void) {
    double arr[SIZE]={20.0, 17.66, 8.2, 15.3, 22.22};

    printf("원래의 arr:\n");

    show_array(arr, SIZE);
    mult_array(arr, SIZE, 2.5);

    printf("함수 호출 후 arr:\n");
    show_array(arr, SIZE);

    return 0;
}

//배열 내용 표시
void show_array(const double ar[], int n) {
    int i;

    for(i=0; i<n; i++) {
        printf("%8.3f", ar[i]);
    }
    putchar('\n');
}

//배열에 숫자를 곱
void mult_array(double ar[], int n, double mult) {
    int i;

    for(i=0; i<n; i++) {
        ar[i]*=mult;
    }
}