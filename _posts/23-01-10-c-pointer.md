---
title:  "[C] 포인터"
excerpt: "포인터에 관하여 정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-01-10
last_modified_at: 2023-01-10
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