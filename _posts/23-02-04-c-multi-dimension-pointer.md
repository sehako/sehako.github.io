---
title:  "[C] 포인터와 다차원 배열"
excerpt: "포인터에 관하여 정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-02-04
last_modified_at: 2023-02-04
---

# 포인터와 다차원 배열
다차원 배열
```c
int arr[2][2];
```
가 있다고 가정해보자. 배열의 이름 `arr`은 배열의 첫 번째 원소의 주소이다. 이 경우 `arr`의 첫 번째 원소 자체가 `int`형 2개 짜리 배열의 주소이다.

따라서 `arr`은 `&arr[0]`과 같은 값을 가진다. 그 다음에 `arr[0]` 자체가 `int`형 2개 배열이기 때문에 `arr[0]`은 `&arr[0][0]`과 같은 값을 가진다. 정수와, 정수 2개짜리 배열이 둘 다 동일한 위치에서 시작하기 떄문에 `arr`과 `arr[0]`은 같은 수치값을 갖는다.

포인터나 주소에 1을 더하는 것은 가리키는 객체의 크기만큼 값을 증가시킨다는 점에서 볼 때, `arr`과 `arr[0]`은 다르다. `arr + 1`과 `arr[0] + 1`은 다른 값을 가지게 된다.

포인터나 주소의 내용을 참조하는 것은, 그것이 가리키는 객체의 값을 제공한다. `arr[0]`은 첫 번쨰 원소 `arr[0][0]`의 주소이므로, `*(arr[0])`은 `arr[0][0]`에 저장되어 있는 하나의 `int` 형 값을 나타낸다. 마찬가지로, `*arr`은 첫 번째 원소 `arr[0]`의 값을 나타낸다. 그러나 `arr[0]` 자체가 하나의 `int` 형 값의 주소로서 `&arr[0][0]`이다. 그래서 `*arr`은 `&arr[0][0]`이다. 

이 두 표현식에 각각 참조 연산자(`*`)를 적용하면, `**arr`이 `*&arr[0][0]`과 같다는 것을 의미한다. 결과적으로 이는 하나의 `int`형 값 `*arr[0][0]`이 된다. `arr`은 주소의 주소이므로 그것으로 보통의 값을 얻으려면 내용을 두 번 참조해야 한다. 주소의 주소 또는 포인터를 가리키는 포인터는 이중 간접 연산(*double indirection*)의 한 예이다.

```c
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
```
```
arr = 000000D2F39FFD30,   arr + 1 = 000000D2F39FFD38
arr[0] = 000000D2F39FFD30,   arr[0] + 1 = 000000D2F39FFD34
*arr = 000000D2F39FFD30,   *arr + 1 = 000000D2F39FFD34
arr[0][0] = 4
*arr[0] = 4
**arr = 4
arr[1][0] = 3
*(*(arr + 2) + 1) = 7
```

출력 결과는 2차원 배열 `arr`의 주소와, 1차원 배열 `arr[0]`의 주소가 같다는 것을 보여준다. 이것은 `&arr[0][0]`과 같다.

그러나 한 가지 차이점이 있다. 우리가 사용한 시스템에서 `int`형은 4바이트이다. 따라서, `arr[0]`은 4바이트 크기의 데이터 객체를 가리킨다. `arr[0] + 1`는 따라서 4바이트 만큼 주소를 이동한다. 그런데 `arr + 1`는 8바이트 만큼 주소를 이동한다. 왜냐면 `arr`은 `int`형 2개짜리 배열이기 떄문에 `arr + 1`의 결과가 8바이트 만큼 움직인 것이다. 

또한 위 예제는 2차원 배열에 저장된 한 값을 얻으려면 그 배열의 이름을 두 번 참조해야 한다는 것을 보여 준다. 간접연산자(`*`)를 두 번 사용하거나, 각괄호 연산자(`[]`)를 두 번 사용해야 한다. 맨 마지막 줄의 `*(*(arr + 1) + 1)`는 `arr[1][1]`과 동등하다. 몇 가지 예시를 정리하자면 다음과 같다.

포인터 표현|해석
|:---:|:---:|
*(arr + 1)|&arr[1][0]
*(arr + 1) + 1|&arr[1][1]
*(*(arr + 1) + 1)|arr[1][1]

## 다차원 배열을 가리키는 포인터

다차원 배열을 다루는 포인터를 선언하는 것은 기존의 포인터 선언법과 다르다. 앞선 예제의 `arr`은 어떤 배열의 첫 번째 원소(`int`형 2개짜리 배열)의 주소이다. 그러므로 포인터는 하나의 `int`형을 가리키는 것이 아닌, `int`형 2개 짜리 배열을 가리켜야 하므로 선언은 다음과 같다

```c
int (* ptr)[2];
```

괄호가 들어간 이유는 각괄호가 간접연산자보다 우선순위가 높기 때문이다. 괄호가 없는 포인터

```c
int * ptr[2];
```

같은 선언은 `ptr`을 포인터 2개짜리 배열로 만든다.

아래 예제는 포인터를 이용하여 배열의 주소값과 값을 출력한다.

```c
#include <stdio.h>

int main(void) {
    int arr[2][2] = {
        {3, 5},
        {7, 4}
    };
    int (* ptr)[2];
    ptr = arr;

    printf("ptr = %p, ptr + 1 = %p\n", ptr, ptr + 1);
    printf("ptr[0] = %p, ptr[0] + 1 = %p\n", ptr[0], ptr[0] + 1); 
    printf("*ptr = %p, *ptr + 1 = %p\n", *ptr, *ptr + 1);
    printf("ptr[0][0] = %d\n", ptr[0][0]);
    printf("*ptr[0] = %d\n", *ptr[0]);
    printf("**ptr = %d\n", **ptr);
    printf("ptr[1][0] = %d\n", ptr[1][0]);
    printf("*(*(ptr + 1) + 1) = %d\n", *(*(ptr + 1) + 1));
    
    return 0;
}
```

```
ptr = 000000F2F73FFDA0, ptr + 1 = 000000F2F73FFDA8
ptr[0] = 000000F2F73FFDA0, ptr[0] + 1 = 000000F2F73FFDA4
*ptr = 000000F2F73FFDA0, *ptr + 1 = 000000F2F73FFDA4
ptr[0][0] = 3
*ptr[0] = 3
**ptr = 3
ptr[1][0] = 7
*(*(ptr + 1) + 1) = 4
```

배열 이름과 포인터 둘 중 어느 것을 가지고 있으면, 개별적인 원소들을 배열 표기를 사용하여 나타낼 수도 있고, 포인터 표기를 사용하여 나타낼 수도 있다.

```c
arr[m][n] == *(*(arr + m) + n)
ptr[m][n] == *(*(ptr + m) + n)
```

## 함수와 다차원 배열
행과 열의 정보를 추적하고자 하는 함수를 구현할 떄, 배열을 전달할 수 있는 적절한 형식매개변수를 선언함으로써 이것을 해결할 수 있다. 정수형 배열을 입력으로 하는 함수의 형식매개변수는 다음과 같이 선언할 수 있다.

```c
void function(int (* ptr)[4]);
//또는
void somefunction(int pt[][4]);
```

두 번쨰 방법의 선언에서 비어 있는 각괄호는 `pt`가 포인터라는 것을 나타낸다. 

다음은 배열의 행의 합, 열의 합, 모든 원소들의 합을 보여주는 예제이다.

```c
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
```

```
1행 합: 20
2행 합: 24
3행 합: 36
1열 합: 17
2열 합: 19
3열 합: 21
4열 합: 23
모든 원소들의 합계: 80
```

`main()`함수의 `arr`과 `ar`이 같은 방식으로 사용된 것도 볼 수 있다. 이는 `arr`과 `ar` 둘 다 `int`형 4개 짜리 배열을 가리키는 포인터형이기 떄문이다.

다음과 같은 선언은 바르게 동작하지 않는다.

```c
int sum2(int ar[][], int rows); //잘못된 선언
```

컴파일러는 배열 표기를 포인터 표기로 변환한다. 이것은 `ar[1] == ar + 1`이라는 것이다. 따라서 다음과 같은 선언

```c
int sum2(int ar[][4], int rows);
```
은 `ar`이 `int`형 4개짜리 배열을 가리킨다고 말하고, `ar + 1`은 그 주소에 `int`형 4개 만큼의 크기(16 byte)를 더하라는 의미히다. 첫 번쨰 각괄호에 숫자를 선언할 수 있지만 이는 컴파일러에 의해 무시된다.

결과적으로 N차원 배열에 대응하는 포인터 선언은 가장 왼쪽의 `[]`를 제외한 모든 `[]`에 값을 제공해야 한다. 첫 번쨰 빈 `[]`가 포인터를 나타내고 나머지 각괄호들은 가리켜지는 객체의 데이터형이기 떄문이다. 

```c
void function(int arr[][5][6][4]);
//이 선언은 다음과 동일하다
void function(int (* arr)[5][6][4]);
```

# 가변 길이 배열 (VLA)
