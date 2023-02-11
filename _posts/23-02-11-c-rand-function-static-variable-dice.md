---
title:  "[C] 메모리 관리"
excerpt: "정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-02-11
---

# 할당된 메모리 malloc() & free()

프로그램을 실행할 때 사용자가 메모리를 할당할 수 있다. 이 작업을 위해 사용하는 도구가 `malloc()` 함수다. 하나의 전달인자로 원하는 만큼의 메모리 바이트 수를 사용한다. 그러면 `malloc()`은 자유 메모리 공간에서 적당한 블록을 찾는다. 하지만 `malloc()`은 메모리를 할당하지만 거기에 이름을 붙이지 않는 익명이고, 할당한 블록의 첫 번째 주소를 리턴한다. 그러므로 포인터 변수에 그 주소를 대입할 수 있다. 

`malloc()` 함수는 배열, 구조체, 기타 등등을 가리키는 포인터를 리턴할 수 있다. 일반적으로 그 리턴값은 데이터형 캐스트를 사용하여 적당한 값으로 변환된다. 그러나 `void`형을 가리키는 포인터 값을 다른 데이터형을 가리키는 포인터에 대입하는 것은 데이터형 충돌로 간주되지 않는다. `malloc()`은 적당한 메모리 공간을 찾지 못하면 NULL 포인터를 리턴한다. 

이해하기 위해 배열을 생성하는 작업에 `malloc()`을 사용해본다. 

```c
double * ptr;
ptd = (double *) malloc (30 * sizeof(double));
```

위 코드는 30개의 double형 값을 저장하기 위한 공간을 요청하고, `ptr`이 그 위치를 가리키도록 설정한다. 여기서 `ptr`이 `double`형 값 30개를 가지는 블록을 가리키는 포인터가 아닌, 하나의 `double`형 값을 가리키는 포인터로 선언된 것에 유념한다. 그리고 배열 이름은 첫 번째 원소의 주소라는 것 또한 유념한다.

`malloc()`은 `free()`와 함께 사용해야 한다. `free()`함수는 바로 전 `malloc()`이 리턴했던 주소를 전달인자로 사용하여, 할당했던 메모리를 해제한다. 그래서, 할당된 메모리의 수명은, `malloc()`의 호출부터 `free()`의 호출로 재사용할 수 있도록 메모리를 해제하는 시점까지다.

`free()`는 항상 `malloc()`에 의해 할당된 메모리를 해제하는 용도로 사용된다. 이 두 함수는 `stdlib.h`에 정의되어 있다. 두 함수에 관한 예제가 존재한다.

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    double * ptd;
    int max;
    int number;
    int i = 0;

    puts("몇 개의 double을 입력할건지?");

    if (scanf_s("%d", &max) != 1) {
        puts("숫자가 정확하게 입력되지 않았습니다.");
        exit(EXIT_FAILURE);
    }
    ptd = (double *) malloc(max * sizeof(double));  //가변 배열 선언

    if (ptd == NULL) {
        puts("메모리 할당에 실패했다.");
        exit(EXIT_FAILURE);
    }

    puts("값들을 입력하시오(q입력시 종료)");

    while (i < max && scanf_s("%lf", &ptd[i]) == 1) {
        ++i;
    }
    printf("입력한 %d개의 값들은 다음과 같다\n", number = i);

    for (i = 0; i < number; i++) {
        printf("%7.2f", ptd[i]);
        if(i % 7 == 6) {
            putchar('\n');
        }
    }
    if(i % 7 != 0) {
        putchar('\n');
    }
    puts("종료");
    free(ptd);  //할당 해제

    return 0;
}
```

```
몇 개의 double을 입력할건지?
5
값들을 입력하시오(q입력시 종료)
123.
22.3
323.1
23
4123.2
입력한 5개의 값들은 다음과 같다
 123.00  22.30 323.10  23.004123.20
종료
```

위 코드에서 `exit()` 함수는 프로그램의 종료를 수행한다. 값 `EXIT_FAILURE` 또한 `stdlib.h` 헤더에 정의되어 있다. 프로그램의 정상적 종료는 `EXIT_SUCCESS`(또는 0), 비 정상적 종료는 `EXIT_FAILURE`이다.

데이터형 캐스트 `(double *)`는 C에서는 선택이고 C++에서는 필수이다. 

## free()의 중요성

정적 메모리의 양은 컴파일 시 고정된다. 그것은 프로그램이 실행되는 동안 변하지 않는다. 자동 변수에 의해 사용되는 메모리의 양은 유동적이다. 그러나 할당된 메모리에 의해 사용되는 메모리의 양은, 사용자가 `free()`를 사용하는 것을 잊으면 계속 커진다. 

```c
int main(void) {
    double glad[2000];
    int i;

    for (i = 0; i < 1000; i++) {
        gobble(glad, 2000);
    }
}

void gobble(double ar[], int n) {
    double * temp = (double *) malloc(n * sizeof(double));
    //free(temp);
}
```
위와 같은 코드가 존재한다면, `temp`는 자동 변수이기 때문에 소멸한다. 그러나 그것이 가리키던 메모리를 해제한다는 것은 아니다. 주소를 더 이상 가지고 있지 않기 떄문에, 16,000바이트의 메모리에는 이제 접근할 수 없다. 

이런 루프가 1000번 실행되면 16,000,000 바이트의 메모리가 메모리 풀에서 미아 상태가 된다. 실제로는 이 상황에 도달 전 프로그램이 메모리를 전부 소모할 것이다. 이런 메모리 누출(memory leak) 문제를 피하기 위해서 `free()`가 중요하다.

# calloc() 함수