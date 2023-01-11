---
title:  "[C] 포인터"
excerpt: "포인터에 관하여 정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-01-10
last_modified_at: 2023-01-11
---

# 포인터
주소를 값으로 가지는 변수(데이터 객체)로, 다음과 같은 형식으로 선언 가능하다.
```c
datatype *name
```
**간단 사용 예시**
```c
#include <stdio.h>

int main(void) {
    int num = 10;
    int * ptr;
    //선언 시에는 간점 연산자(*)와 변수명 사이에 스페이스를 넣는다
    int val;

    ptr = &num;
    val = *ptr;
    printf("val = %d", val);
    
    return 0;
}
```
```
val = 10
```

## 함수 간 포인터 사용

```c
#include <stdio.h>

void interchange(int * u, int * v);
int main(void) {
    int x = 5, y = 10;

    printf("교환 전 x = %d, y = %d\n", x, y);
    interchange(&x, &y);
    printf("교환 후 x = %d, y = %d\n", x, y);

    return 0;
}

void interchange(int * u, int * v) {
    int temp;

    temp = *u;
    *u = *v;
    *v = temp;
}
```
```
교환 전 x = 5, y = 10
교환 후 x = 10, y = 5
```
어떻게 이것이 가능했을까?
```
interchange(&x, &y);
```
해당 라인은 값(value)을 전달하는 대신, x와 y의 **주소**를 전달한다. 따라서 `interchange()`의 형식매개변수가 주소를 값으로 가진다.

함수가 어떤 동작이나 계산을 위한 값을 요구한다면, 포인터가 굳이 필요가 없다. 하지만 함수가 호출 함수에 있는(다른 지역에 있는) 변수를 변경할 필요가 있다면 포인터를 형식매개변수로 사용하는 함수는 아주 유용할 것이다.

# 포인터와 배열
배열의 표기는 실제로는 포인터의 변장된 사용에 불과하다.
```c
array == &array[0] // 배열의 이름은 첫 번째 원소의 주소
```
```c
#include <stdio.h>
#define SIZE 4

int main(void) {
    short array[SIZE];
    short * pti;
    short index;
    double array_d[SIZE];
    double * ptf;

    pti = array;
    ptf = array_d;
    printf("%23s %15s\n", "short", "double");
    for (index = 0; index < SIZE; index++) {
        printf("포인터 + %d: %10p %10p\n", index, pti + index, ptf + index);
    }

    return 0;
}
```
```
                  short          double
포인터 + 0: 0000009193BFF610 0000009193BFF5F0
포인터 + 1: 0000009193BFF612 0000009193BFF5F8
포인터 + 2: 0000009193BFF614 0000009193BFF600
포인터 + 3: 0000009193BFF616 0000009193BFF608
```

포인터에 1을 더한다는 것은 하나의 기억 단위를 더한다는 것과 같다. 배열의 경우에 다음 원소의 주소로 증가한다는 것을 의미한다. 따라서 해당 표현식도 성립한다
```c
array + 2 == &array[2]
*(array + 2) == array[2]
//따라서
array[n] == *(array + n)    
```
종합적으로 배열이 개별적인 원소에 접근하고, 그 값을 얻는 데 포인터를 사용할 수 있다. 
```c
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
```
따라서 다음과 같은 사용도 가능하다. 하지만 이런 표기 방법에 딱히 이점은 없다. 위 예시의 요점은, 배열 표기와 포인터 표기가 동등한 방법이라는 것이다.

# 함수, 배열, 포인터
배열이 가진 원소들의 총합을 리턴하는 함수를 작성한다고 가정해보자
```c
int sum(int * ar, int n);

int main(void) {...}

int sum(int * ar, int n) {
    int i;
    int total = 0;

    for (i = 0; i < n; i++) {
        total += ar[i];
    }
    return total;
}
```
이때, 함수 프로토타입이나 함수 정의 부분에서 int * ar을 int ar[]로 대체할 수 있다.
```c
int sum(int ar[], int n);
```
프로토타입에서는 이름도 생략이 가능하다. 다음 4개의 함수 프로토타입 선언은 모두 같은 표현이다.
```c
int sum(int * ar, int n);
int sum(int *, int);
int sum(int ar[], int n);
int sum(int [], int);
```
## 배열과 포인터의 크기
```c
#include <stdio.h>
#define SIZE 10

int sum(int ar[], int n);

int main(void) {
    int array[SIZE] = {20, 14, 26, 34, 25, 45, 11, 53, 16, 32};
    long answer;

    answer = sum(array, SIZE);
    printf("총 합 = %ld\n",answer);
    printf("array의 크기는 %zd바이트\n", sizeof array);

    return 0;
}

int sum(int ar[], int n) {
        int i;
    int total = 0;

    for (i = 0; i < n; i++) {
        total += ar[i];
    }

    printf("ar의 크기는 %zd바이트\n", sizeof ar);
    
    return total;
}
```
```
ar의 크기는 8바이트
총 합 = 276
array의 크기는 40바이트
```

`array`는 4바이트 `int` 원소 10개를 가진 크기가 총 40바이트라는 것은 이치에 맞는다. 그러나 `ar`의 크기는 겨우 8바이트이다. 이것은 `ar`이 배열 그 자체가 아닌 `array`의 첫 번째 원소를 가리키는 포인터이기 때문이다. 시스템이 8바이트 주소를 사용하기 떄문에 포인터 변수의 크기가 8바이트이다.

## 포인터 매개변수의 사용
배열을 조작하는 함수는 시작과 끝을 알아야 한다. `sum()` 함수는 끝 부분을 알리기 위해서 정수 매개변수를 사용했지만, 포인터 매개변수 두 개를 이용하는 방법도 존재한다.
```c
#include <stdio.h>
#define SIZE 10

int sump(int * start, int * end);

int main(void) {
    int array[SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int answer;

    answer = sump(array, array + SIZE);

    printf("합 = %d\n", answer);

    return 0;
}

int sump(int * start, int * end) {
    int total = 0;

    while (start < end) {
        total += *start; //total에 포인터가 가리키는 변수의 값을 덧셈
        start++; //다음 원소를 가리키도록 1 증가
    }

    return total;
}
```
```
합 = 55
```

`sump` 함수는 `sum` 함수와는 다르게 `while` 반복문을 이용하였다. 이것은 `end`가 배열에 있는 마지막 원소 바로 다음 위치를 가리키기 때문에 사용 가능한 방법이다. C는 배열 생성을 위한 공간 할당에 있어서 배열의 끝 바로 다음의 첫 번째 위치를 가리키는 포인터가 유효하다는 것을 보장한다. 이것은 루프에서 `start`가 얻는 최종값이 `end`이기 떄문에, 이와 같은 구조를 유효하게 만든다.(*past-the-end pointer*)

또한 덧셈 연산과 주소증가 부분을 한 라인으로 만들 수 있다.
```c
total += *start++;
```
단항 연산자 `*`과 `++`는 우선순위는 같지만 오른쪽에서 왼쪽으로 결합한다. 따라서 이것은 `++`가 `*start`가 아닌 `start`에 적용된다는 것을 의미한다. 즉 위의 표현식은 다음과 같다.
```c
total += *(start++);
```
이런 우선순위의 미묘한 차이를 보여주는 예시가 존재한다.
```c
#include <stdio.h>
int data[2] = {100, 200};
int data2[2] = {300, 400};

int main(void) {


    int * p1, * p2, * p3;

    p1 = p2 = data;
    p3 = data2;
    printf("*p1 = %d, *p2 = %d, *p3 = %d\n", *p1, *p2, *p3);
    printf("*p1++ = %d, *++p2 = %d, (*p3)++ = %d\n", *p1++, *++p2, (*p3)++);
    printf("*p1 = %d, *p2 = %d, *p3 = %d\n", *p1, *p2, *p3);

    return 0;
}
```
```
*p1 = 100, *p2 = 100, *p3 = 300
*p1++ = 100, *++p2 = 200, (*p3)++ = 300
*p1 = 200, *p2 = 200, *p3 = 301
```

# 포인터 연산
