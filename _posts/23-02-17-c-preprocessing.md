---
title:  "[C] 전처리기"
excerpt: "정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-02-17
---

# 전처리기

## #define

![image](/assets/images/c_image_05.png)

`#define` 전처리기는 다음과 같은 사용법들이 존재한다.

```c
#define TEN 10
#define STRING "Hello World!"
#define HUNDRED TEN * TEN
#define PRNT printf("X: %d\n", x);
```

마지막 라인은 처음 사용된 형태로, C의 표현식까지도 메크로로 표현할 수 있다는 것을 나타낸다.

### 전달인자 사용

함수같은 매크로를 만들 수 있다.

```c
#define SQUARE(X) X * X
```

이 경우 `X`가 실제로 함수의 전달인자처럼 사용된다.

### # 연산자

```c
#define PRNT(X) printf("#X: %d\n", X);
```

함수형 메크로의 문자열 내에 전달인자의 변수 이름을 표현하고자 할 때 다음과 같이 `#`를 통하여 표현이 가능하다.

### ## 연산자

두 개의 토큰을 하나의 토큰으로 결합한다.

```c
#define XNAME(n) x ## n

...

XNAME(4)
```

```
x4
```

### 가변 전달인자 매크로

```c
#define PR(...) printf(__VA_ARGS__)
```

매크로 정의 전달인자 리스트의 마지막 전달인자로 생략 기호를 사용할 수 있다. 생략 기호를 사용하면, 대체 리스트에서 그것을 대체하는 것을 지정하기 위해서 미리 정의된 메크로 `__VA_ARGS__`를 사용할 수 있다.

### 매크로 vs 함수

이는 시간 절약 vs 메모리 절약으로 갈라진다. 매크로는 인라인 코드를 만들어 프로그램 안에 문장을 넣는다. 또한 변수의 데이터형에 신경 쓸 필요가 없다. 반면에 함수는 프로그램에 단 한 번 나타나므로 메모리 공간을 적게 차지하지만, 함수가 있는 곳으로 프로그램 제어가 매번 이동하고 복귀해야 하므로 인라인 코드보다 시간이 더 걸린다.

일반적으로 간단한 함수들은 매크로로 사용하는 경향이 있다.

```c
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
```

## include

파일의 내용을 현재 파일에 포함시키는 전처리기다. 

```c
#include <stdio.h>
#include "stuff.h"
#include "/user/repo/stuff.h"
```

`<>`는 하나 또는 그 이상의 표준 시스템 디렉토리에서 그 파일을 지시한다.

`""`는 지시된 경로에서 (커스텀 헤더)파일을 찾는다.

파일을 포함시키는 이유는 컴파일러가 요구하는 정보를 가지고 있기 때문이다. 일반적으로 `stdio.h`에는 `EOF`, `NULL`, `getchar()`, `putchar()`등의 정의가 들어있고 C 입출력에 관한 함수들의 프로토타입이 들어 있다.

## #undef

이전에 이루어진 `#define` 정의를 무효화한다. 

```c
#define LIMIT 100
#undef LIMIT  //LIMIT의 정의(100)가 해제된다.
```

## #ifdef, #else, #endif

```c
#ifdef MAVIS
  #include "stuff.h"  
  #define STABLES 5
#else
  #include "stuf.h"
  #define STEBLES 5
#endif
```

위와 같은 코드는 `MAVIS`가 전처리기에 의해 정의된 경우에 `#else` 또는 `#endif` 둘 중 하나가 먼저 나타나는 위치까지 모든 전처리기 지시자와 모든 C 코드를 컴파일하라고 지시한다. 

식별자가 정의되지 않았는지 확인하는 `#ifndef`도 존재한다.

일반적으로 헤더 파일의 중복 포함을 막기 위해서 보통 사용된다. 

## #if, #elif

`#if` 다음에 오는 상수 표현식이 0이 아니면 참으로 간주된다. 

```c
#if SYS == 1
  #include "stuff.h"
#elif SYS == 2
  #include "vax.h"
#elif SYS == 3
  #include "mac.h"
#endif
```

`defined` 키워드를 이용하여 다음과 같이 만들 수 있다. 해당 키워드는 전달인자가 정의되어 있으면 1, 정의되어 있지 않으면 0을 리턴한다.

```c
#if defined (IBMPC)
  #include "stuff.h"
#elif defined (VAX)
  #include "vax.h"
#elif defined (MAC)
  #include "mac.h"
#else
  #include "general.h"
#endif
```

## Generic Selection
`#define` 매크로 선언의 일부로, `switch`와 비슷한 부분이 있다.

```c
#define MYTYPE(X) __Generic((X), \
int: "int",\
float: "float",\
double: "double",\
default: "other")
```

이렇게 정의된 범용 선택 식은 주어진 데이터형에 따라 문자열을 출력한다.

이 외에도 많은 전처리기가 존재한다.