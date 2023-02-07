---
title:  "[C] 포인터"
excerpt: "포인터에 관하여 정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-01-10
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

다음 예시는 포인터로 가능한 연산들을 보여준다.
```c
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
    printf("ptr1 + 4 = %p, *(ptr1 + 3) = %d\n", ptr1 + 4, *(ptr1 + 3));
    
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
```

## 대입
포인터에 주소를 대입하는 것이다.
```c
ptr1 = urn;
ptr2 = &urn[2]; 
```

## 역참조
`*` 연산자는 그것이 참조하는 주소에 저장되어 있는 값을 구한다.
```c
printf("포인터 값, 역참조된 포인터가 가리키는 값, 포인터의 주소\n");
printf("ptr1 = %p,  *ptr1 = %d, &ptr1 = %p\n", ptr1, *ptr1, &ptr1);
```
위 코드에서 `*ptr1`이 역참조 부분이다.
```
포인터 값, 역참조된 포인터가 가리키는 값, 포인터의 주소
ptr1 = 0000000ED75FFD40,  *ptr1 = 1, &ptr1 = 0000000ED75FFD38
```

## 포인터 주소 얻기
포인터 역시 변수이기 떄문에, `&` 연산자를 이용하여 포인터 자체가 어디에 저장되어 있는지 알 수 있다.

## 포인터와 정수 연산
```c
ptr3 = ptr1 + 4;
printf("\n 포인터에 정수 덧셈\n");
printf("ptr1 + 4 = %p, *(ptr1 + 3) = %d\n", ptr1 + 4, *(ptr1 + 3));
```

```
포인터에 정수 덧셈
ptr1 + 4 = 0000000ED75FFD50, *(ptr1 + 3) = 4
```
포인터에 정수를 더하거나, 정수에 포인터를 더하기 위해 `+` 연산자를 사용할 수 있다. 정수는 항상 포인터가 가리키는 데이터형의 바이트 수만큼 곱해진다. 그러고 나서 그 결과가 최초의 주소에 더해진다. 이것이 `ptr1 + 4`를 `&urn[4]`와 같게 만든다.

`-` 연산자를 사용해서 뺄셈 또한 가능하다. 
```c
//포인터 정수 뺄셈
printf("\n포인터에서 정수 뺄셈\n");
printf("ptr3 = %p, ptr3 - 2 = %p\n", ptr3, ptr3 - 2);
```
```
포인터에서 정수 뺄셈
ptr3 = 00000051EB3FF9A0, ptr3 - 2 = 00000051EB3FF998
```

포인터 덧셈과 마찬가지로, 이것은 배열 내에서의 이전 위치를 정수만큼 이동시키는 것과 같은 효과를 낸다. 증감연산자와 전위모드 및 후위모드 또한 동일하게 사용 가능하다.
```c
    ptr1++; //포인터 증가
    printf("\nptr1++ 수행 결과\n");
    printf("ptr1 = %p, *ptr1 = %d, &ptr1 = %p\n", ptr1, *ptr1, &ptr1);

    ptr2--; //포인터 감소
    printf("\n--ptr2를 수행한 후의 값\n");
    printf("ptr2 = %p, *ptr2 = %d, &ptr2 = %p\n", ptr2, *ptr2, &ptr2);
```

```
ptr1++ 수행 결과
ptr1 = 00000051EB3FF994, *ptr1 = 2, &ptr1 = 00000051EB3FF988

--ptr2를 수행한 후의 값
ptr2 = 00000051EB3FF994, *ptr2 = 2, &ptr2 = 00000051EB3FF980
```

## 포인터 사이의 차 구하기

같은 배열을 가리키는 두 포인터 사이의 차를 구하면, 포인터가 가리키는 원소들의 거리가 결과값으로 반환된다.

```c
//포인터 뺄셈
printf("\n포인터에서 다른 포인터 뺄셈\n");
printf("ptr2 = %p, ptr1 = %p, ptr2 - ptr1 = %td\n", ptr2, ptr1, ptr2 - ptr1);
```

```
포인터에서 다른 포인터 뺄셈
ptr2 = 00000051EB3FF998, ptr1 = 00000051EB3FF990, ptr2 - ptr1 = 2
```

여기서 2는 두 포인터가 2개의 `int`형 크기만큼 떨어져 있는 객체들을 가리킨다는 것을 의미한다.

## 포인터 비교

두 포인터가 같은 데이터형을 가리키는 경우, 두 포인터의 값을 비교하기 위해 관계 연산자를 사용할 수 있다.

## 포인터 연산 주의 사항

포인터를 증감시킬 때 포인터가 여전히 배열 원소를 가리키는지 컴퓨터는 검사하지 않는다. C에서는, 어떤 배열이 주어졌을 떄 배열의 원소를 가리키는 포인터와 마지막 원소 바로 다음의 위치를 가리키는 포인터가 유효한 포인터라고 보장한다. 이 범위를 벗어나게 포인터를 연산한다면 어떤 결과가 나오는지 정의되지 않았다.

또한 배열 원소를 가리키는 포인터는 포인터가 가리키는 내용을 참조할 수 있다. 그러나 마지막 바로 다음 위치를 가리키는 포인터가 유효하다고 보장은 하지만 그 포인터가 가리키는 내용을 참조할 수 있다고 보장하지 않는다.

# 포인터와 초기화
반드시 기억해야 할 주의 사항으로, 초기화되지 않은 포인터의 내용을 역참조하지 않는 것이다.

```c
int * ptr;
*pt = 5;
```
다음과 같은 코드가 있다면 이 코드는 상당히 문제가 많은 코드다. 왜냐면 `ptr`이 초기화된 상태가 아니기 때문에 무작위 값을 가지기 떄문이다. 따라서 5가 어디에 저장될지 알 수 없다. 이것은 데이터나 코드를 덮어쓰거나, 프로그램을 먹통으로 만들 수 있다. 따라서 포인터를 생성하면 포인터 자체를 저장하기 위한 메모리만 할당(데이터를 저장하기 위한 메모리는 할당되지 않는 다는 것)된다는 것을 유념해야 한다.

# 배열 내용의 보호

기본 데이터형(`int` 등)을 다룰 때 일반적인 규칙은, 값을 변경할 필요가 있는 경우 포인터를 전달한다는 것이다. 배열의 경우 무조건 포인터를 전달해야 하는데, 그 이유는 효율성 때문이다. 함수가 어떤 배열을 값으로 전달한다면 원본 배열의 복사본을 저장할 만큼의 공간을 할당하고, 복사본에 원본의 내용을 복사해야 한다. 

하지만 포인터를 사용하는 방식은 원본의 훼손을 야기할 수 있다. ANSI C에서는 이러한 위험을 피하기 위해 함수의 의도가 배열 내용을 변경하지 않는다면 프로토타입과 함수 정의에서 형식매개변수를 선언할 떄 `const`를 사용하는 것이다. 

```c
int sum(const int ar[], int n); //프로토타입
...
int sum(const int ar[], int n) {
    int i;
    int total=0;

    for (i=0; i<n; i++) {
        total+=ar[i];
    }
    return total;
}
```

이것은 컴파일러에게 그 함수가 ar이 가리키는 배열을 상수 데이터처럼 다루어야 한다고 지시한다. 여기서 한 가지 중요한 것은, `const`를 이런식으로 사용할 때 함수에 사용되는 원본 배열은 상수일 필요가 없다는 것이다. 

```c
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
```

```
원래의 arr:
  20.000  17.660   8.200  15.300  22.220
함수 호출 후 arr:
  50.000  44.150  20.500  38.250  55.550
```

`const`는 놓는 위치에 따라서 다른 용도로도 사용이 가능하다.
```c
int arr[5] = {1,2,3,4,5};
int * const ptr = arr;
ptr = &rates[2];   //허용 X
*ptr = 6    //arr[0]의 값이 6으로 변경

const int * const pt = arr;
pt = &arr[2];   //허용 X
*pt = 6 //허용 X
```