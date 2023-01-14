#include <stdio.h>

int main(void) {
    int urn[5] = {1, 2, 3, 4, 5};
    int * ptr1, * ptr2, * ptr3;

    ptr1 = urn; //포인터에 주소 대입
    ptr2 = &urn[2]; //포인터에 주소 대입
    //역참조된 포인터를 참조하여 포인터의 주소를 얻는 방식

    printf("포인터 값, 역참조된 포인터가 가리키는 값, 포인터의 주소\n");
    printf("ptr1 = %p,  *ptr1 = %d, &ptr1 = %p\n", ptr1, *ptr1, &ptr1);

    //포인터 덧셈
    ptr3 = ptr1 + 4;
    printf("\n 포인터에 정수 덧셈\n");
    printf("ptr1 + 4 = %p, *(ptr4 + 3) = %d\n", ptr1 + 4, *(ptr1 + 3));
    
    ptr1++; //포인터 증가
    printf("\nptr1++ 수행 결과\n");
    printf("ptr1 = %p, *ptr1 = %d, &ptr1 = %p\n", ptr1, *ptr1, &ptr1);

    ptr2--; //포인터 감소
    printf("\n--ptr2를 수행한 후의 값\n");
    printf("ptr2 = %p, *ptr2 = %d, &ptr2 = %p\n", ptr2, *ptr2, &ptr2);
    
    --ptr1; //최초의 값으로 복원
    ++ptr2; //최초의 값으로 복원
    printf("\n두 포인터를 최초의 값으로 복원한다\n");
    printf("ptr1 = %p, ptr2 = %p\n", ptr1, ptr2);

    //포인터 뺄셈
    printf("\n포인터에서 다른 포인터 뺄셈\n");
    printf("ptr2 = %p, ptr1 = %p, ptr2 - ptr1 = %td\n", ptr2, ptr1, ptr2 - ptr1);

    //포인터 정수 뺄셈
    printf("\n포인터에서 정수 뺄셈\n");
    printf("ptr3 = %p, ptr3 - 2 = %p\n", ptr3, ptr3 - 2);

    return 0;
}