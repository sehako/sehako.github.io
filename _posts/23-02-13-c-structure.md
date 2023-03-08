---
title:  "[C] 구조체"
excerpt: " "

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

위 코드는 두 개의 문자 배열과 하나의 `float`형 변수로 구성된다. C에 존재하는 모든 데이터형은 구조체의 멤버가 될 수 있다. 실제 데이터 객체를 생성하지 않고, 무엇이 그러한 구조체를 구성하는지만 나타낸다. `book`은 선택사항인 태그로 그 구조체를 참조하는 데 사용할 수 있는 약식 레이블이다. 그러므로, 나중에 다음과 같은 선언을 할 수 있는 것이다.

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

각 초기화자는 당연하게도 구조체 멤버의 데이터형과 일치해야 한다. 

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

특정 멤버에 가장 마지막으로 제공된 값이 그 멤버의 값이 된다. 예를 들어,

```c
struct book library = {
    .value = 30000,
    .author = "James",
    10000
};
```

구조체 선언에서 이런 경우 마지막 값 10000은 `value`에 대입된다. 따라서 이전에 제공된 값은 새롭게 대체된다.

## 구조체 멤버에 접근

구조체는 하나의 특별배열과 비슷하다. 구조체의 개별 멤버에 접근하려면 멤버 연산자인 도트(`.`)를 사용할 수 있다. 

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

구조체를 가리키는 포인터를 사용하여 좋은 점은 세 가지가 있다.

1. 구조체를 가리키는 포인터가 구조체 자체보다 다루기가 쉽다.
2. 구조체를 가리키는 포인터는 함수에 전달할 수 있다.
3. 많은 데이터 표현들이 다른 구조체를 가리키는 포인터를 멤버로 가지는 구조체를 사용한다.

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
    struct person man[2] = {
        {
            {"James", "Hallow"},
            "Ramen",
            "Trainer",
            50000
        }, 
        {
            {"Bit", "Coin"},
            "Sushi",
            "Trader",
            100000   
        }
    };

    struct person * ptt;
    ptt = &man[0];
    
    printf("주소 #1: %p #2: %p\n", &man[0], &man[1]);
    printf("포인터 #1: %p #2: %p\n", ptt, ptt + 1);
    printf("ptt -> income: %f, (*ptt).income: %f\n", ptt -> income, (*ptt).income);
    ptt++;
    printf("ptt -> food: %s, ptt -> handle.last: %s\n", ptt -> food, ptt -> handle.last);

    return 0;
}
```

```
ptt -> income: 50000.000000, (*ptt).income: 50000.000000
ptt -> food: Sushi, ptt -> handle.last: Coin
```

```c
struct person * ptt;

ptt = &man[0];
```

위와 같은 선언은 `person` 구조체를 가리키는 포인터를 선언하고 그 포인터에 구조체 변수를 가리키도록 초기화한다. 

## 포인터를 사용하여 멤버에 접근하기

포인터를 사용한 접근의 방법으로 새로운 연산자인 `->`가 새롭게 등장했다. 이는 구조체 이름 뒤에 도트 연산자가 오는 것과 동일한 효과이다.

```c
ptt -> income == man.income
```

구조체의 한 멤버의 값을 나타내는 두 번쨰 방법은 다음과 같은 시퀀스를 따른다. `ptt == &man[0]`이면, `*ptt == man[0]`이다. 다음과 같이 대체하여 사용할 수 있다.

```c
man[0].income == (*ptt).income
```

다음의 것들은 모두 동등하다.

```c
man.income == (*ptt).income == ptt -> income //ptt == &man일 떄
```

# 함수에 구조체 알리기

최신 컴파일러는 구조체 자체를 전달해도 되고, 구조체를 가리키는 포인터를 전달할 것인지를 사용자가 선택할 수 있다. 세 가지 방법이 존재한다.

## 구조체의 멤버 전달하기

```c
#include <stdio.h>

#define FUNDLEN 50

struct funds {
    char bank[FUNDLEN];
    double bankfund;
    char save[FUNDLEN];
    double savefund;
};

double sum(double, double);

int main(void) {
    struct funds stan = {
        "ABC",
        3500,
        "Locker",
        6500
    };

    printf("Stan: %f\n", sum(stan.bankfund, stan.savefund));

    return 0;
}

double sum(double x, double y) {
    return (x + y);
}
```

## 구조체의 주소 사용하기

```c
#include <stdio.h>

#define FUNDLEN 50

struct funds {
    char bank[FUNDLEN];
    double bankfund;
    char save[FUNDLEN];
    double savefund;
};

double sum(const struct funds *);

int main(void) {
    struct funds stan = {
        "ABC",
        3500,
        "Locker",
        6500
    };

    printf("Stan: %f\n", sum(&stan));

    return 0;
}

double sum(const struct funds * money) {
    return (money -> bankfund + money -> savefund);
}
```

포인터가 가리키는 값의 내용을 함수가 변경하면 안되기에 `money`가 `const`를 가리키는 포인터로 선언되었다.

## 전달인자로 구조체 전달

```c
#include <stdio.h>

#define FUNDLEN 50

struct funds {
    char bank[FUNDLEN];
    double bankfund;
    char save[FUNDLEN];
    double savefund;
};

double sum(struct funds money);

int main(void) {
    struct funds stan = {
        "ABC",
        3500,
        "Locker",
        6500
    };

    printf("Stan: %f\n", sum(stan));

    return 0;
}

double sum(struct funds money) {
    return (money.bankfund + money.savefund);
}
```

# 구조체의 그 밖의 특성
한 구조체를 다른 구조체에 대입하는 것을 허용한다. `n_data`와 `o_data`가 같은 데이터형의 구조체라면, 다음과 같이 할 수 있다.

```c
o_data = n_data;
```

이것은 `o_data`의 각 멤버에 `n_data`의 대응하는 각 멤버의 값이 대입되게 한다. 이것은 멤버가 배열이어도 동작한다. 또한 같은 데이터형의 구조체라면, 초기화 또한 가능하다.

```c
struct names right_field = {"ABC", "DEF"};
struct names captain = right_field;
```

그리고 구조체를 함수의 리턴값으로도 리턴할 수 있다. 이런 경우, 피호출 함수로부터 호출 함수로 구조체 정보를 전달할 수 있다. 그리고 구조체 포인터는 양방향 커뮤니케이션을 허용한다. 

```c
#include <stdio.h>
#include <string.h>

#define NLEN 30

struct namect {
    char fname[NLEN];
    char lname[NLEN];
    int letters;
};

void getinfo(struct namect *);
void makeinfo(struct namect *);
void showinfo(const struct namect *);

int main(void) {
    struct namect person;

    getinfo(&person);
    makeinfo(&person);
    showinfo(&person);

    return 0;
}

void getinfo(struct namect * pst) {
    printf("이름 입력: ");
    gets_s(pst -> fname, NLEN);
    printf("성씨 입력: ");
    gets_s(pst -> lname, NLEN);
}

void makeinfo(struct namect * pst) {
    pst -> letters = strlen(pst -> fname) + strlen(pst -> lname);
}

void showinfo(const struct namect * pst) {
    printf("%s %s: %d", pst -> fname, pst -> lname, pst -> letters);
}
```

```
이름 입력: James
성씨 입력: Hallow
James Hallow: 11
```

구조체 자체를 함수에 전달하고 리턴 값으로 구조체를 리턴하는 방법도 있다.

```c
#include <stdio.h>
#include <string.h>

#define NLEN 30

struct namect {
    char fname[NLEN];
    char lname[NLEN];
    int letters;
};

struct namect getinfo(void);
struct namect makeinfo(struct namect);
void showinfo(const struct namect);

int main(void) {
    struct namect person;

    person = getinfo();
    person = makeinfo(person);
    showinfo(person);

    return 0;
}

struct namect getinfo(void) {
    struct namect temp;
    printf("이름 입력: ");
    gets_s(temp.fname, NLEN);
    printf("성씨 입력: ");
    gets_s(temp.lname, NLEN);

    return temp;
}

struct namect makeinfo(struct namect info) {
    info.letters = strlen(info.fname) + strlen(info.lname);

    return info;
}

void showinfo(struct namect info) {
    printf("%s %s: %d", info.fname, info.lname, info.letters);
}
```

```
이름 입력: James
성씨 입력: Hallow
James Hallow: 11
```

## 구조체 vs 구조체 포인터

구조체 포인터를 전달인자로 사용하는 방식은 주소 하나만 전달하기 떄문에 빠르다. 그러나, 데이터의 보호가 부족하다는 단점이 있다. 피호출 함수에서 수행하는 동작이 데이터를 변질시킬 수도 있다. 따라서 `const` 제한자를 사용하는 것이 좋을 것이다. 

구조체를 전달인자로 전달하는 방식의 장점은, 함수가 원본 데이터의 복사본을 가지고 작업한다는 것이다. 따라서 원본 데이터를 가지고 작업하는 것보다 안전하고 또한 프로그래밍 스타일이 좀 더 명확하다. 그러나 구조체 안에 있는 한 두개의 멤버만 사용하는 함수에 구조체 전체를 전달하는 것은 낭비다.

따라서 대부분의 사용법에서 구조체 포인터를 함수의 전달인자로 사용한다. 의도치 않은 데이터 변경은 `const`를 사용하여 방지하면 되기 때문이다.

## 구조체와 문자 포인터

```c
struct names {
    char * first;
    char * last;
};
```

다음과 같이 선언할 수 있다. 이 코드를 이용하여 다음과 선언하면

```c
struct names jh = {"James", "Hallow"};
```

이것은 구조체 내에 문자가 저장되는 것이 아니다. 이 경우 문자열 상수를 저장하는 곳에 문자열들이 저장되고, 포인터는 그것을 가리킨다. 이런 구조체의 경우 2개의 주소를 가지는 만큼의 바이트를 차지한다. 

이런 방식이 문제가 될 때는 다음과 같다.

```c
struct names ff;
scanf("%s", ff.last);
```

위 경우 포인터에 의해 주어지는 주소에 그 문자열을 넣는다. 이것은 초기화되지 않아서 그 주소는 어떤 값이라도 가질 수 있다. 그래서 프로그램은 엉뚱한 곳에 문자열을 저장할 수도 있다. 따라서 구조체 안에 문자열을 저장해야 한다면, 문자 배열을 사용한다. 

## 구조체, 포인터 malloc()

문자열을 다루기 위해 구조체 안에 포인터를 사용하는 경우가 괜찮을 때가 있다. `malloc()`을 이용하여 메모리를 할당하고, 그 주소를 포인터에 보관하는 것이다. 이 방법은 문자열을 저장하는 데 꼭 필요한 만큼의 메모리를 `malloc()`에게 요청할 수 있다는 것이다. 

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SLEN 81

struct namect {
    char * fname;
    char * lname;
    int letters;
};

void getinfo(struct namect *);
void makeinfo(struct namect *);
void showinfo(const struct namect *);
void cleanup(struct namect *);

int main(void) {
    struct namect person;

    getinfo(&person);
    makeinfo(&person);
    showinfo(&person);
    cleanup(&person);

    return 0;
}

void getinfo(struct namect * pst) {
    char temp[SLEN];
    printf("이름 입력: ");
    gets_s(temp, SLEN);

    //이름을 저장할 메모리를 할당
    pst -> fname = (char *) malloc(strlen(temp) + 1);
    //할당된 메모리에 이름을 복사 
    strcpy(pst -> fname, temp);
    printf("성씨 입력: ");
    gets_s(temp, SLEN);
    pst -> lname = (char *) malloc(strlen(temp) + 1);
    strcpy(pst -> lname, temp);
}

void makeinfo(struct namect * pst) {
    pst -> letters = strlen(pst -> fname) + strlen(pst -> lname);
}

void showinfo(const struct namect * pst) {
    printf("%s %s: %d", pst -> fname, pst -> lname, pst -> letters);
}

void cleanup(struct namect * pst) {
    free(pst -> fname);
    free(pst -> lname);
}
```

```
이름 입력: James
성씨 입력: Hallow
James Hallow: 11
```

## 복합 리터럴과 구조체

복합 리터럴은 배열 뿐 아니라 구조체에도 사용할 수 있다. 

```c
#include <stdio.h>

#define MAXTITL 41
#define MAXAUTL 31

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book readfirst;
    int score;

    printf("테스트 스코어 입력: ");
    scanf_s("%d", &score);

    if(score >= 84) {
        readfirst = (struct book) {
            "How to C",
            "James",
            50000
            };
    }
    else {
        readfirst = (struct book) {
            "How to Python",
            "Hallow",
            30000
        };
    }

    printf("할당된 독서:\n");
    printf("%s, %s, %f", readfirst.title, readfirst.author, readfirst.value);

    return 0;
}
```
테스트 스코어 입력: 0
할당된 독서:
How to Python, Hallow, 30000.000000
테스트 스코어 입력: 100
할당된 독서:
How to C, James, 50000.000000
```

```c
readfirst = (struct book) {
    "How to Python",
    "Hallow",
    30000
};
```

복합 리터럴은 함수에 전달인자로 사용할 수도 있다. 함수가 구조체를 기대한다면 복합 리터럴을 실전달인자로 전달할 수 있다.

```c
struct rect {double x; double y;};
double rect_area(struct rect r){return r.x * r.y};

...

double area;
area = rect_area((struct rect) {10.5, 20.0});
```

함수가 주소를 기대한다면, 복합 리터럴의 주소를 전달할 수도 있다.

```c
struct rect {double x; double y;};
double rect_areap(struct rect * rp) {return rp -> x * rp -> y;};

...
double area;
area = rect_areap(&(struct rect) {10.5, 20.0});
```

함수의 바깥에 있는 복합 리터럴은 정적 수명을 가진다. 어떤 블록 안에 있는 복합 리터럴은 자동 수명을 가진다. 초기화자 리스트에 적용되는 것과 동일한 신택수 규칙이 복합 리터럴에 적용된다.

## 플렉서블 배열 멤버

이 기능은 마지막 멤버가 특별한 속성을 가지는 배열인 구조체를 선언할 수 있도록 한다. 첫 번쨰 특별한 속성은, 지금 당장 존재하지 않는 배열이라는 것이다. 두 번쨰는, 플렉서블 배열 멤버를 마치 그것이 존재하는 것처럼 그리고 필요한 만큼 원소를 가지고 있는 것처럼, 적당한 코드와 함께 사용할 수 있다는 것이다.

플렉서블 배열 멤버를 선언하는 규칙은 다음과 같다.

- 플렉서블 배열 멤버는 구조체의 마지막 멤버가 되어야 한다. 
- 다른 멤버가 최소한 하나 있어야 한다.
- 플렉서블 배열은, 각괄호 안이 비워놓는다는 것을 제외하고, 보통의 배열처럼 선언한다.

```c
struct flex {
    int count;
    double average;
    double scores[];    //플렉서블 배열 멤버
};
```

`struct flex`형의 변수를 선언하면 `scores`를 어떤 것에도 사용할 수 없다. 그것을 위한 메모리가 할당되지 않았기 떄문이다. 이것은 `struct flax`형의 변수를 선언하려고 의도하는 것이 아니다. 그 대신, `struct flax`형을 가리키는 포인터를 선언하고, `struct flax`형의 정규 내용들을 저장할 공간과 플렉서블 배열 멤버를 위해 원하는 만큼의 추가 공간을 할당받기 위해 `malloc()`을 사용하라 의도하는 것이다.

```c
struct flax *pf;

pf = malloc(sizeof(struct flex) + 5 * sizeof(double));
```

이것으로 `count`, `average`, `double`형 값 5개짜리 배열을 저장할 수 있는 메모리 덩어리가 확보되고 포인터 `pf`를 사용하여 이들 멤버에 접근할 수 있다.

```c
pf -> count = 5;
pf - > scores[2] = 18.5;
```

```c
#include <stdio.h>
#include <stdlib.h>

struct flex {
    size_t count;
    double average;
    double scores[];
};

void showFlex(const struct flex * p);

int main(void) {
    struct flex * pf1, * pf2;
    int n = 5;
    int i;
    double tot = 0.0;

    pf1 = malloc(sizeof(struct flex) + n * sizeof(double));
    pf1 -> count = n;
    
    for(i = 0; i < n; i++) {
        pf1 -> scores[i] = 20.0 - i;
        tot += pf1 -> scores[i];
    }
    pf1 -> average = tot / n;
    showFlex(pf1);

    n = 9;
    tot = 0.0;
    pf2 = malloc(sizeof(struct flex) + n * sizeof(double));
    pf2 -> count = n;

    for(i = 0; i < n; i++) {
        pf2 -> scores[i] = 20.0 - i / 2.0;
        tot += pf2 -> scores[i];
    }

    pf2 -> average = tot / n;
    showFlex(pf2);
    free(pf1);
    free(pf2);

    return 0;
}

void showFlex(const struct flex * p) {
    int i;
    
    printf("스코어: ");
    
    for(i = 0; i < p -> count; i++) {
        printf("%g ", p -> scores[i]);
    }
    
    printf("\n평균: %g\n", p -> average);
}
```

```
스코어: 20 19 18 17 16
평균: 18
스코어: 20 19.5 19 18.5 18 17.5 17 16.5 16
평균: 18
```

유연한 배열 멤버를 갖춘 구조체는 일부 특별한 조건이 있다. 먼저 복사할 때 구조체 대입을 하지 않는 것이다.

```c
struct flex *pf1, *pf2;

*pf2 = *pf1;    //이렇게 하지 않는 것
```

위와 같은 코드는 구조체의 유연성 없는 멤버들만 복사할 것이다. 대신 `memcpy()`함수를 사용한다. 

두 번쨰로, 이러한 종류의 구조체는 값에 의한 구조체를 전달하는 함수와 함께 사용하지 않는 것이다. 값에 의한 전달인자를 전달하는 것은 대입과 같다. 대신, 구조체의 주소를 전달하는 함수를 사용한다.

세 번째로, 구조체를 유연성 있는 배열의 요소로서 또는 다른 구조체의 멤버로 사용하지 않는 것이다. 

## Anonymous 구조체

이름이 명명되지 않은 구조체 멤버이다. 이것이 어떻게 작동하는지 보려면 먼저 중첩된 구조체를 위한 다음 단계를 고려할 필요가 있다.

```c
struct names {
    char first[20];
    char last[20];
};

struct person {
    int id;
    struct names name;
};

struct person ted = {8483, {"Ted", "Grass"}};

```

여기서 `name` 멤버는 중첩된 구조체이다. `"Ted"`에 접속하는 `ted.name.`같은 표현을 사용할 수 있다.

```c
puts(ted.name.first);
```

중첩된 익명의 멤버 구조를 정의할 수 있다.

```c
struct person {
    int id;
    struct {char first[20]; char last[20];};    //anonymous 구조체
}
``

위 같은 구조체도 다음과 같은 초기화가 가능하다.

```c
struct person ted = {8483, {"Ted", "Grass"}};
```

그러나 접근은 `person` 멤버인 것처럼 `first`와 같이 멤버 이름을 사용하는 것으로 단순화된다.

## 구조체의 배열을 사용하는 함수

함수를 사용하여 구조체의 배열을 처리해야 한다고 가정하자. 배열 이름은 배열의 주소와 동등하므로, 함수에 전달될 수 있다. 그리고 그 함수는 구조체 템플릿에 접근할 필요가 있다. 

(깃허브 빌드 오류 때문에 스크린샷으로 대체)

![image](/assets/images/c_image_06.png)

```
94054.440000
```

배열 이름 `jones`는 배열의 주소이다. 또한 이것은 그 배열의 첫 번째 원소인 구조체 jones[0]의 주소이기도 하다. 그러므로, 포인터 `money`는 처임에 다음과 같이 주소가 주어진다.

```c
money = &jones[0];
```
`money`는 `jones`배열의 첫 번째 원소를 가리키므로, `money[0]`은 그 배열의 첫 번째 원소를 나타내는 또 다른 이름이다. 마찬가지로 `money[1]`은 그 배열의 두 번째 원소를 나타내는 또 다른 이름이다. 각 원소는 `funds` 구조체다. 그래서 각각 `.`연산자를 이용하여 구조체 멤버에 접근할 수 있다.

## 구조체의 내용을 파일에 저장

구조체는 다양한 정보를 저장할 수 있기 때문에, DB를 구축하는데 쓰이는 도구이기도 하다. 구조체를 정의해 정보를 대입하고 그것을 파일형태로 저장하고, 파일에서 정보를 검색하기를 원할 것이다. 이런 DB는 그런 데이터 객체가 여러 개 존재한다. 하나의 구조체에 저장되는 정보의 총 집합을 레코드(record)라 부르고, 각각의 개별 항목을 필드(field)라고 부른다.

가장 간단한 형태는 `fprintf()`를 사용하는 것이다. 하지만 멤버의 수가 많다면 다루기 어려워진다. 또한, 하나의 필드가 어디서 끝나고 어디서 시작되는지를 프로그램에 알려 주어야 하므로, 정보 검색에 어려움을 가져온다. 

좀 더 괜찮은 해결책은 `fread()`와 `fwrite()`를 사용하여 구조체 크기 단위로 읽고 쓰는 것이다. 이 함수들은 프로그램이 사용하는 것과 같은 바이너리 표현을 사용하여 읽고 쓴다. 

```c
struct book {
    ...
};

struct book structure;
fwrite(&structure, sizeof (struct book), 1, pbooks);
```

위 코드는 `pbooks`가 파일 스트림을 나타낼 떄 `&structure` 구조체의 시작 주소로 가서, 그 주소의 모든 바이트를 `pbooks`에 연결된 파일에 하나(1)만 복사한다. `ffread()`는 구조체 크기만큼의 데이터 덩어리를 파일로부터 `&structure`가 가리키는 위치에 복사한다. 이 두 함수는 한 번에 하나의 레코드를 읽고 쓴다.

바이너리 표현 방식에서 데이터를 저장하는 한 가지 단점은 서로 다른 시스템마다 다른 바이너리 표현 방식을 사용할 수 있다는 것이다. 따라서 데이터 파일은 이식성을 가지지 못할 수 있다. 심지어 같은 시스템이어도 다른 컴파일러 세팅이라면 다른 바이너리 레이아웃이 될 수 있다.

```c
#include <stdio.h>
#include <stdlib.h>

#define MAXTITL 40
#define MAXAUTL 40
#define MAXBKS 10

struct book {
    char title[MAXTITL];
    char author[MAXAUTL];
    float value;
};

int main(void) {
    struct book library[MAXBKS];
    int count = 0;
    int index, filecount;
    FILE * pbooks;
    int size = sizeof (struct book);

    if((pbooks = fopen("book.dat", "a + b")) == NULL) {
        fputs("book.bat 파일을 열 수 없음\n", stderr);
        exit(1);
    }
    
    rewind(pbooks); //파일의 시작으로 이동

    while(count < MAXBKS && fread(&library[count], size, 1, pbooks) == 1) {
        if(count == 0) {
            puts("book.dat에 들어 있는 내용: ");
        }
        printf("%s by %s: %f\n", library[count].title, library[count].author, library[count].value);
        count++;
    }
    filecount = count;

    if(count == MAXBKS) {
        fputs("book.dat 파일이 가득 차 있음\n", stderr);
        exit(2);
    }

    puts("새 도서 제목을 입력: ");
    puts("끝내려면 [enter]");

    while(count < MAXBKS && gets(library[count].title) != NULL && library[count].title[0] != '\0') {
        puts("저자명 입력: ");
        gets_s(library[count].author, MAXAUTL);
        puts("정가 입력: ");
        scanf_s("%f", &library[count++].value);

        while(getchar() != '\n') {
            continue;   //입력 라인을 깨끗이 비움
        }

        if(count < MAXBKS) {
            puts("다음 도서 제목 입력: ");
        }
    }

    if (count > 0) {
        puts("다음은 소장하고 있는 도서 목록이다:");

        for(index = 0; index < count; index++) {
            printf("%s by %s: %f\n", library[index].title, library[index].author, library[index].value);
        }
        fwrite(&library[filecount], size, count - filecount, pbooks);
    }
    else {
        puts("책이 한 권도 없음!");
    }

    puts("끝");
    fclose(pbooks);
}
```

두 번 실행한 결과다

```
새 도서 제목을 입력:
끝내려면 [enter]
How To C
저자명 입력:
James
정가 입력:
50000
다음 도서 제목 입력:
How to Python
저자명 입력:
Hallow
정가 입력:
30000
다음 도서 제목 입력:

다음은 소장하고 있는 도서 목록이다:
How To C by James: 50000.000000
How to Python by Hallow: 30000.000000
끝

book.dat에 들어 있는 내용:
How To C by James: 50000.000000
How to Python by Hallow: 30000.000000
새 도서 제목을 입력:
끝내려면 [enter]
How to Java
저자명 입력:
Peter
정가 입력:
60000
다음 도서 제목 입력:

다음은 소장하고 있는 도서 목록이다:
How To C by James: 50000.000000
How to Python by Hallow: 30000.000000
How to Java by Peter: 60000.000000
끝
```

이 처럼 저번에 저장한 데이터가 다음 실행에도 남아있다는 것을 알 수 있다.

파일을 열기 위한 모드 `"a + b"`는 `a+`가 프로그램 파일 전체를 읽고, 파일의 끝에 데이터를 추가할 수 있게 한다. `b`는 바이너리 파일 포멧을 사용하도록 지정하는 방법이다. 

`rewind()`는 파일을 처음부터 읽을 수 있도록 파일 위치 포인터를 파일의 시작으로 옮긴다. 

위 예제는 사용하지 않는 구조체의 부분도 함께 저장되기에 기억 공간의 낭비가 우려된다. 이 구조체의 크기는 `2 x 40 x sizeof(char) + sizeof(float)`인데, 입력 항목 중 어느 것도 그 공간을 모두 사용하지 않는다. 그러나 각 데이터 덩어리가 같은 크기이므로 데이터 검색이 쉽다는 장점또한 존재한다. 이런 공간 낭비를 해결하기 위해서 가변 크기 레코드를 사용할 수 있다. 

# 구조체의 다음 단계

구조체의 중요한 용도 중 하나는 바로 새로운 데이터형 만들기이다. 컴퓨터 사용자들은 어떤 문제들을 해결하는 데 훨씬 효율적인 데이터형을 개발해왔다. 큐, 바이너리 트리, 힙, 헤시 테이블, 그래프 등과 같은 이름을 가지고 있다. 이러한 데이터형들은 구조체를 연결하여 만든다. 일반적으로, 각 구조체는 한두 개의 데이터 항목과, 같은 유형의 다른 구조체를 가리키는 한두 개의 포인터를 가진다. 이 포인터들은 한 구조체를 다른 구조체에 연결하여, 사용자가 구조체들의 집합 전체를 탐색할 수 있는 경로를 제공한다.

나중에 C와 자료구조 책을 정리할 것이다. 