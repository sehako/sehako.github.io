---
title:  "[C] 구조체 & 그 밖의 데이터형"
excerpt: "정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-02-13
---

# 구조체

```c
#include <stdio.h>
#include <string.h>
#define MAXTITL 41
#define MAXAUTL 31

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book library;    //library를 book형 변수로 선언

    printf("도서 제목 입력: ");
    gets_s(library.title, MAXTITL);
    printf("저자명 입력: ");
    gets_s(library.author, MAXAUTL);
    printf("정가 입력: ");
    scanf_s("%f", &library.value);
    printf("%s, %s, %f", library.title, library.author, library.value);
    printf("종료");

    return 0;
}
```
```
도서 제목 입력: How to C language
저자명 입력: James
정가 입력: 50000
How to C language, James, 50000.000000
종료
```

위와 같이 여러 변수들을 하나로 종합시켜 `struct` 형태로 만들어 구조체로 사용할 수 있다.

## 구조체 선언의 설정

구조체의 선언은 다음과 같다.

```c
struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};
```

위 코드는 두 개의 문자 배열과 하나의 `float`형 변수로 구성된다. C에 존재하는 모든 데이터형은 구조체의 맴버가 될 수 있다. 실제 데이터 객체를 생성하지 않고, 무엇이 그러한 구조체를 구성하는지만 나타낸다. `book`은 선택사항인 태그로 그 구조체를 참조하는 데 사용할 수 있는 약식 레이블이다. 그러므로, 나중에 다음과 같은 선언을 할 수 있는 것이다.

```c
struct book library;
```

위 코드는 `book` 구조체 설계를 사용하는 구조체 변수 `library`를 선언한다.

## 구조체 변수의 정의

구조체라는 단어는 두 가지 의미로 사용된다. 첫 번째 의미는, "구조체 설계"를 의미한다. 구조체 설계는 그 데이터를 표현하는 방법을 컴파일러에게 알려 주지만, 컴퓨터는 그 데이터를 위한 기억 공간을 할당하지 않는다. 두 번쨰 의미는, 그 다음 단계로서 "구조체 변수"를 만드는 것이다.

```c
struct book library;
```

이 문장을 만났을 때, 컴파일러는 변수 `library`를 생성한다. `book` 템플릿(태그)을 사용하여 컴파일러는 구조체 내에 정의된 변수들을 위한 기억 공간을 할당한다. 이 기억 공간은 `library`라는 단일 이름으로 한 덩어리를 이룬다. 

구조체 변수의 선언에서 `struct book`은 간단한 변수의 선언에서 `int`나 `float`가 하는 것과 같은 역할을 한다. 두 개의 `struct book`형 변수와 그러한 구조체를 가리키는 포인터를 다음과 같이 선언할 수 있다.

```c
struct book doyle, panshin, * ptbook;
```

각 구조체 변수는 각각의 구조체 내 정의된 변수들을 갖는다.  `ptbook`은 `book` 구조체를 가리킬 수 있다. 사실상, 구조체 선언은 새로운 데이터형을 만든다.

```c
struct book library;
```

이와 같은 선언은 다음과 같다.

```c
struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
} library;
```

따라서, 구조체를 선언하는 과정과 구조체 변수를 정의하는 과정이 한 단계로 결합될 수 있다.

```c
struct {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
} library;
```

이런식으로 태그를 사용하지 않고 구조체를 선언할 수도 있다. 구조체 템플릿을 여러 번 사용할 계획이라면, 태크 형식을 사용하는 것이 좋다.

## 구조체의 초기화

구조체의 초기화는 배열의 초기화와 비슷한 신택스를 사용한다.

```c
struct book library = {
    "How to C language",
    "James",
    50000
};
```

각 초기화자는 당연하게도 구조체 맴버의 데이터형과 일치해야 한다. 

구조체는 지정 초기화 또한 가능하다.

```c
struct book library = { .value = 50000};
//지정 초기화자는 순서에 상관없이 사용할 수 있다.
struct book library = { 
    .value = 20000,
    .author = "James",
    .title = "How to C language"
    };
```

특정 맴버에 가장 마지막으로 제공된 값이 그 맴버의 값이 된다. 예를 들어,

```c
struct book library = {
    .value = 30000,
    .author = "James",
    10000
};
```

구조체 선언에서 이런 경우 마지막 값 10000은 `value`에 대입된다. 따라서 이전에 제공된 값은 새롭게 대체된다.

## 구조체 맴버에 접근

구조체는 하나의 특별배열과 비슷하다. 구조체의 개별 맴버에 접근하려면 맴버 연산자인 도트(`.`)를 사용할 수 있다. 

```c
scanf_s("%f", &library.value);
```

# 구조체의 배열

동일한 여러 구조체 변수를 관리하기 위해서 구조체를 배열 형식으로 선언할 수 있다.

```c
#include <stdio.h>
#include <string.h>

#define MAXTITL 40
#define MAXAUTL 40
#define MAXBKS 50

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book library[MAXBKS];
    int count = 0;
    int index;

    printf("도서 제목 입력: ");

    while(count < MAXBKS && gets_s(library[count].title, MAXTITL) != NULL && library[count].title[0] != '\0') {
        printf("저자 이름 입력: ");
        gets_s(library[count].author, MAXAUTL);
        printf("정가 입력: ");
        scanf_s("%f", &library[count++].value);

        while(getchar() != '\n') {
            continue;
        }
        if(count < MAXBKS) {
            printf("다음 타이틀 입력(끝내려면 [enter] 입력): ");
        }
    }

    if(count > 0) {
        printf("소장하고 있는 도서들의 목록:\n");
        for(index = 0; index < count; index++) {
            printf("%s, %s, %f\n", library[index].title, library[index].author, library[index].value);
        }
    }
    else {
        printf("책이 없습니다.\n");
    }

    return 0;
}
```

```c
struct book library[MAXBKS];
```

위 코드는 `MAXBKS`개의 원소를 가지는 배열 `library`를 선언한다. 배열의 각 원소는 `book`형의 구조체이다. 접근은 다음과 같이 할 수 있다.

```c
library[index].title
```

# 중첩된 구조체
하나의 구조체에 또 다른 구조체를 포함(중첩)시키는 것이 편리할 떄가 있다. 

```c
#include <stdio.h>

#define LEN 20

struct names {
    char first[LEN];
    char last[LEN];
};

struct person {
    struct names handle;
    char food[LEN];
    char job[LEN];
    float income;
};

int main(void) {
    struct person man = {
        {"James", "Hallow"},
        "Ramen",
        "Trainer",
        50000
    };

    printf("%s, %s, %s, %s, %f", man.handle.last, man.handle.first, man.food, man.job, man.income);

    return 0;
}
```

```
Hallow, James, Ramen, Trainer, 50000.000000
```

중첩된 구조체를 선언하는 방법과 그런 중첩된 구조체에 접근할 때 신택스를 주의해서 보자.

# 구조체를 가리키는 포인터