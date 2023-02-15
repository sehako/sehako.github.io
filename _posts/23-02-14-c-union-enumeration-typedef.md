---
title:  "[C] 공용체, 열거형, typedef"
excerpt: "정리"

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-02-13
---

# 공용체
같은 메모리 공간에 서로 다른 데이터형들을 (동시에 저장하지는 않음) 저장할 수 있게 한다. 공용체의 대표적인 용도는, 순서가 규칙적이지 않고 미리 알 수도 없는 데이터형들의 혼합을 저장하도록 설계된 테이블이다. 공용체의 배열을 사용하면, 같은 크기의 단위들로 구성된 배열을 만들고, 각 단위에 다양한 데이터형을 저장할 수 있다. 

공용체는 구조체와 같은 방식으로 설정한다. 공용체 템플릿과 공용체 변수가 있다. 공용체는 한 번에 정의되거나, 공용체 태그를 사용하여 두 번에 걸쳐 정의될 수 있다.

```c
union hold {
    int digit;
    double bigfl;
    char letter;
};
```

구조체와 다르게 공용체는 하나의 `int`형 값 또는 하나의 `double`형 값 또는 하나의 `char`형 값을 저장할 수 있다. 다음은 `hold`형 공용체 변수 3개를 정의하는 예이다.

```c
union hold fit;
union hold save[10];
union hold * pu;
```

공용체는 값을 하나만 가지고, 초기화 방법 세 가지 중에서 하나를 선택할 수도 있다. 한 공용체를 같은 유형의 다른 공용체로 초기화하거나, 공용체의 첫 멤버를 초기화하거나, 지정 초기화자를 사용할 수도 있다.

```c
union hold valA;
valA.letter = 'A';
union hold valB = valA; //공용체 끼리의 초기화
union hold valC = {88}; //공용체의 digit 멤버를 초기화
union hold valD = (.bigfl = 110.0); //지정 초기화자로 초기화
```

## 공용체 사용

```c
fit.digit = 20; //fit에 20이 저장
fit.bigfl = 2.0; //fit이 2.0으로 대체됨
fit.letter = 'h'; // fit이 'h'로 대체됨
```

공용체는 한 번에 하나의 값만 저장된다는 것을 기억한다. 

구조체를 가리키는 포인터에 사용하는 방식과 동일하게 `->` 연산자를 사용할 수 있다.

```c
pu = &fit;
x = pu -> digit; // x = fit.digit
```

다음과 같은 시퀀스는 사용하면 안된다.

```c
fit.letter = 'A';
flnum = 3.02 * fit.bigfl;
```

공용체를 사용하는 하나의 용도는, 저장되는 정보가 멤버들 중 하나에 의존하는 구조체 안에 사용하는 것이다. 

## Anonymous 공용체

무명의 구조체 같이 작동한다. 무명의 공용체는 구조체나 공용체의 이름없는 멤버 공용체이다. 다음과 같은 예시가 존재한다.

```c
struct owner {
    char socsecurity[12];
    ...
};

struct leasecompany {
    char name[40];
    char headquarters[40];
    ...
};

struct car_data {
    char make[15];
    int status; //0 = owned, 1 = leased
    union {
        struct owner owncar;
        struct leasecompany leasecar;
    };
    ...
};
```

만약 `flits`가 `car_data` 구조체라면, `flits.owncar.socsecurity`를 `flits.ownerinfo.owncar.socsecurity`대신 사용할 수 있게 된다.

# 열거형

정수 상수를 나타내는 기호 이름을 선언할 수 있다. `enum` 키워드를 이용하여 새로운 데이터형을 만들고, 그 데이터형이 가질 수 있는 값들을 지정할 수 있다. (`enum` 상수들은 사실상 `int` 형이므로 `int`형을 사용할 수 있는 곳이라면 어디라도 그것을 사용할 수 있음) 해당 키워드의 목적은 프로그램의 가독성을 높이는 것이다. 신택스는 구조체에 사용되는 것과 비슷하다.

```c
enum spectrum {red, orange, yellow, green, blue, violet};
enum spectrum color;
```

첫 번째 선언은 `spectrum`을 태그 이름으로 설정한다. 이것은 `enum spectrum`을 데이터형 이름으로 사용할 수 있게 한다. 두 번째 선언은 `color`를 그 데이터형의 변수로 만든다. 중괄호 내 식별자들은 `spectrum` 변수가 가질 수 있는 가능한 값들을 열거한다. 이런 기호화된 상수들은 '열거된 상수'들이라고 부른다. 

```c
int c;
color = blue;

if(color == yellow) {
    ...;
}

for(color = red; color <= violet; color++) {
    ...;
}
```

`enum`을 이용하여 위 코드를 구성할 수 있다.

## enum 상수

앞서 열거된 상수는 `int`형 상수라고 하였다. 

```c
printf("%d, %d\n", red, orange);
```

위 코드의 출력은 다음과 같다.

```
0, 1
```

열거된 리스트에 있는 상수들은, 기본 설정으로 정수값 0, 1, 2, 3 등으로 대입된다.

따라서 정수형 상수를 사용할 수 있는 어느 곳이라면 어디든지 열거된 상수를 사용할 수 있다.

## 지정된 값

열거된 상수들이 사용자가 원하는 정수값을 갖도록 지정할 수 있다.

```c
enum levels {low = 100, medium = 500, high = 2000};
```

한 상수에만 값을 지정하고 그 이후 상수에는 지정하지 않으면, 그 다음 상수들에는 연속적으로 수가 매겨진다.

```c
enum feline {cat, dog = 10, bird, tiger};
```

`cat`은 디폴트 값으로 0이 되고, 그 이후로는 10, 11, 12가 매겨진다.  

## enum 사용법

열거형의 목적은 프로그램의 가독성을 높이고 유지하기 쉼게 하는 것이다. 

```c
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LEN 30

enum spectrum {red, orange, yellow, green, blue, violet};
const char * colors[] = {"red", "orange", "yellow", "green", "blue", "violet"};

int main(void) {
    char choice[LEN];
    enum spectrum color;
    bool color_is_found = false;

    puts("컬러 입력(끝내려면 빈 라인): ");

    while(gets(choice) != NULL && choice[0] != '\0') {
        for(color = red; color <= violet; color++) {
            if(strcmp(choice, colors[color]) == 0) {
                color_is_found = true;
                break;
            }
        }
        if(color_is_found) {
            switch(color) {
                case red:
                puts("1. red");
                break;
                case orange:
                puts("2. orange");
                break;
                case yellow:
                puts("3. yellow");
                break;
                case green:
                puts("4. green");
                break;
                case blue:
                puts("5. blue");
                break;
                case violet:
                puts("6. violet");
                break;
            }
        }
        else {
            printf("%s는 정의되어 있지 않음\n", choice);
        }
        color_is_found = false;
        puts("다음 컬러 입력(끝내려면 빈 라인): ");
    }
    puts("종료");

    return 0;
}
```

```
컬러 입력(끝내려면 빈 라인):
red
1. red
다음 컬러 입력(끝내려면 빈 라인):
white
white는 정의되어 있지 않음
다음 컬러 입력(끝내려면 빈 라인):

종료
```

## 공유 이름공간

C는 이름공간(namespace)라는 용어를 사용하여, 어떤 이름이 프로그램의 어느 부분들에 알려지는지를 나타낸다. 범위(scope)도 그 개념의 일부다. 같은 이름이지만 범위가 다른 두 변수는 충돌하지 않는다. 같은 범위에 있는 같은 이름의 두 변수는 충돌한다. 

이름공간에는 범주(category)라는 측면이 있다. 한 특정 범위에 있는 구조체 태그, 공용체 태그, 열거형 태그는 같은 이름공간을 공유한다. 그 이름공간은 보통의 변수들이 사용하는 이름공간과 다르다. 이것은, 같은 범위에 있는 같은 이름의 변수와 태그는 에러를 일으키지 않고 사용할 수 있다는 것을 의미한다. 그러나 같은 범위에 있는 같은 이름의 두 태그 또는 같은 이름의 두 변수는 선언할 수 있다.

```c
struct rect {double x; double y;};
int rect;   //C에서 충돌하지 않는다.
```

하지만 위와같은 방법은 혼란을 가져올 수 있다. 또한 C++은 이것을 허용하지 않는다.

# typedef

어떤 데이터형에 원하는 이름을 부여할 수 있는 데이터 관련 고급 기능이다. `#define`과 비슷하지만 차이점이 있다.

- 값이 아닌 데이터형에만 기호 이름을 부여할 수 있도록 제한된다.
- 해석은 전처리기가 아닌 컴파일러가 수행한다. 

```c
typedef unsigned char BYTE;
```

위와 같은 선언은 `BYTE`를 `char`형 변수인 것처럼 정의한다.

```c
BYTE x, y[10], * z;
```

선언의 범위는 명령문이 어디에 있느냐에 따라서 지역 선언인지 전역 선언인지 달라진다.

이런 데이터형에 이름을 부여하는 것은 가독성과 이식성을 높여준다. 예를 들어, `sizeof` 연산자가 리턴하는 데이터형을 나타내는 `size_t`형과, `time()` 함수가 리턴하는 값의 데이터형을 나타내는 `time_t`형을 설명했다. C 표준은 이 두 데이터형이 정수형을 리턴한다. 그러나 정확히 어떤 데이터형을 결정할 지는 컴퍼일러에 달려 있다. 따라서 `time_t`와 같은 새로운 데이터혈 이름을 만들고, 각각의 컴파일러들이 `typedef`을 사용하여 그것을 구체적인 어떤 데이터형으로 설정하도록 하였다.

`typedef`의 일부 기능은 `#define`과 함께 사용될 수 있다.

```c
#define BYTE unsigned char
```

이런 경우 전처리기가 `BYTE`를 `unsigned char`로 대체한다. `#define`과 함께 사용할 수 없는 기능이 한 가지 있다.

```c
typedef char * STRING;
```

키워드 `typedef`이 없다면, 이것은 `STRING`이 `char`형을 가리키는 포인터를 의미하는 식별자가 된다. 따라서, 다음과 같은 선언

```c
STRING name, sign;
```

다음과 같이 선언하는 것과 같다.

```c
char * name, * sign;
```

다음과 같이 정의했다고 가정하자

```c
#define STRING char *
```

그러면 다음과 같은 선언은

```c
STRING name, sign;
```

다음과 같이 번역될 것이다.

```c
char * name, sign;
```

이 경우에는 `name`만 포인터가 된다.

`typedef` 구조체와 함께 사용할 수도 있다.

```c
typedef struct complex {
    float real;
    float imag;
} COMPLEX;
```

위 코드의 경우 `struct complex` 대신에 `COMPLEX`형을 사용하여 복소수를 나타낼 수 있다. `typedef`를 사용하는 한 가지 이유는, 자주 쓰이는 데이터형을 위한 편리하고 쉽게 인식할 수 있는 이름을 만드는 것이다.

```c
typedef struct {double x; double y;} rect;
```

`typedef`를 사용하여 구조체형에 이름을 붙일 때 태그를 생략할 수 있다.

```c
typedef struct {double x; double y;} rect;

rect r1 = {3.0, 6.0};
rect r2;

//다음과 같이 해석됨

struct {double x; double y;} r1 = {3.0, 6.0};
struct {double x; double y;} r2;

r2 = r1;
```

동일한 멤버들을 가지는 (이름과 데이터형이 모두 일치하는) 두 구조체가 태그 없이 선언된다면, C는 두 구조체가 동일한 데이터형이라고 판단한다. 따라서 마지막 문장은 유효하다.

`typedef`를 사용하는 두 번째 이유는, 복잡한 데이터형을 `typedef` 이름으로 간단하게 표현할 수 있기 때문이다.

```c
typedef char (* FRPTC()) [5];
```

위 코드는 `FRPTC`를 `char`형 5개짜리 배열을 가리키는 포인터를 리턴하는 함수의 데이터형으로 만든다.

# 복잡한 선언

어떤 선언을 만들 때, 그 이름(또는 식별자)에 변경자를 붙여 선언을 변경할 수 있다.

변경자|의미
:---:|:---:
*|포인터
()|함수
[]|배열

한 번에 하나의 변경자를 사용할 수 있다. 

변경자를 적용하는 순서에 따라서 변수가 달라진다.

**변경자 순서**

1. 배열을 나타내는 `[]`과, 함수를 나타내는 `()`는 우선순위가 같고, 간접 연산자 `*`보다 우선순위가 높다. 
2. `[]`과 `()`는 왼쪽에서 오른쪽으로 결합한다. 

```c
int board[8][8];    //int형 배열의 배열
int ** ptr; //int형을 가리키는 포인터를 가리키는 포인터
int * risks[10];    //int형을 가리키는 포인터 10짜리 배열
int (* rusks)[10];  //int형 10개짜리 배열을 가리키는 포인터
int * oof[3][4];    //int형을 가리키는 포인터들의 3 x 4 배열
int (* uuf)[3][4];  //int형 3 x 4 배열을 가리키는 포인터
int (* uof[3])[4];  //int형 4개짜리 배열을 가리키는 포인터 3개짜리 배열
```

# 함수와 포인터

함수를 가리키는 포인터를 선언할 수 있다. 함수 포인터는 또 다른 함수에 전달인자로 사용되어, 그 함수에게 어떤 함수를 사용할 것인지 알린다. 

예를 들어, C 라이브러리에 있는 `qsort()` 함수는 원소들을 비교하는 데 사용할 함수를 사용자가 제대로 지정해 주기만 하면, 배열의 종류에 상관 없이 작업할 수 있도록 설계되어 있다. 이 목적을 위해서 전달인자들 중의 하나로 함수를 가리키는 포인터를 사용한다. 그러고 나면 `qsort()` 함수는, 원소들의 데이터형이 무엇이든 간에 배열을 정렬할 수 있다.

함수 포인터는 함수 코드가 시작되는 위치의 주소를 가질 수 있다. 그 다음, 데이터 포인터와 마찬가지로 함수 포인터도 그것이 가리키는 함수형을 선언해야 한다. 함수형을 지정하려면, 함수 시그너쳐(?)를 지정한다. 다시 말해, 함수의 리턴형과 함수 매개변수의 데이터형들을 제공하면 된다.

```c
void ToUpper(char *);   //문자열을 대문자로 변환
```

다음과 같은 프로토타입은 하나의 `char *` 전달인자를 사용하고 `void`형을 리턴하는 함수이다. 이 함수형을 가리키는 포인터는 다음과 같다.

```c
void (*pf)(char *); 
```

이 선언을 만든느 가장 간단한 방법은, 함수 이름 `ToUpper()`를 `(*pf)`로 대체하는 것이다. 그래서 특정 함수형을 가리키는 포인터를 선언하려면, 그 유형의 함수를 먼저 선언하고 나서 함수 이름을 `(*pf)`로 대체하여 함수 포인터를 선언한다. 괄호를 제거하면 우선순위 규칙에 의해 다른 선언이 되어버린다는 것에 유의한다.

```c
void *pf(char *);   //pf는 포인터를 리턴하는 함수가 된다.
```

함수 포인터를 만들면, 그 유형을 가진 함수들의 주소를 함수 포인터에 대입할 수 있다.

```c
void ToUpper(char *);
void ToLower(char *);
int round(double);
void (*pf)(char *);

...

pf = ToUpper;
pf = ToLower;
pf = round; //함수형이 다르기 때문에 무효
pf = ToLower(); //주소가 아니어서 무효
```

함수 포인터를 사용하여 함수에 접근할 수 있다. 그런데 논리적으로 서로 일치하지 않는 두 가지 신택스 규칙이 있다.

```c
void ToUpper(char *);
void ToLower(char *);
void (*pf)(char *);
char mis[] = "Nina Metier";

...

pf = ToUpper;
(*pf)(mis); //mis에 ToUpper 적용

pf = ToLower;
pf(mis);    //mis에 ToLower 적용
```

두 가지 접근 방식이 모두 사리에 맞는 것처럼 보인다. 첫 번째 방식의 경우, `pf`가 `ToUpper` 함수를 가리키기 때문에 `*pf`는 `ToUpper` 함수이다. 따라서 `(*pf)(mis)`는 `ToUpper(mis)`와 같다. 두 번째 방식의 경우, 함수의 이름은 포인터이기 때문에 사용자는 포인터와 함수 이름을 교환하여 사용할 수 있다. 그러므로  `pf(mis)`는 `ToLower(mis)`와 같다.

함수 포인터의 가장 일반적인 용도의 하나는 바로 함수의 전달인자로 사용하는 것이다.

```c
void show(void (* fp)(char *), char * str);
```
위와 같은 함수 프로토타입은, `fp`(함수 포인터)와 `str`(데이터 포인터)이라는 2개의 전달인자를 선언한다. 따라서 아래와 같은 함수 호출을 만들 수 있다.

```c
show(ToLower, mis);
show(pf, mis);
```

`show()`는 자신에게 전달되는 함수 포인터를 호출하기 위해 `fp()`또는 `(*fp)()` 신택스를 사용한다.

```c
void show(void (* fp)(char *), char * str) {
    (*fp)(str);
    puts(str);
}
```

위와 같은 코드는 `show()`가 가리키는 함수를 사용하여 문자열 `str`을 반환하고 나서 변환된 문자열을 표시한다.

리턴값이 있는 함수들은 다른 함수들에 두 가지 방식으로 전달될 수 있다.

```c
function1(sqrt); // sqrt 함수의 주소 전달
function2(sqrt(4.0));    //sqrt 함수의 리턴값 전달
```

아래 예제는 다양한 변환 함수들을 사용하고, 메뉴를 처리하는 몇 가지 유용한 테크닉을 볼 수 있다.

```c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LEN 81

char showmenu(void);
void eatline(void); //라인의 끝까지 읽음
void show(void (* fp)(char *), char * str);
void ToUpper(char *);   //문자열을 대문자로 변환
void ToLower(char *);   //문자열을 소문자로 변환
void Transpose(char *); //대소문자를 교차 변환
void Dummy(char *); //원본 문자열을 그대로 둚

int main(void) {
    char line[LEN];
    char copy[LEN];
    char choice;
    void (*pfun)(char *);   //char 전달인자 사용, 리턴값 없는 함수를 가리킴
    
    puts("문자열 입력(끝내려면 빈 라인): ");
    
    while(gets(line) != NULL && line[0] != '\0') {
        while((choice = showmenu()) != 'n') {
            switch(choice) {
                case 'u':
                pfun = ToUpper;
                break;
                case 'l':
                pfun = ToLower;
                break;
                case 't':
                pfun = Transpose;
                break;
                case 'o':
                pfun = Dummy;
                break;
            }
            strcpy(copy, line);
            show(pfun, copy);
        }
        puts("문자열 입력(끝내려면 빈 라인): ");
    }
    puts("종료");

    return 0;
}

char showmenu(void) {
    char ans;
    puts("메뉴에서 원하는 직업 선택:");
    puts("u) 대문자 변환    l)소문자 변환");
    puts("t) 대소문자 교차 변환 o)원본을 그대로");
    puts("n) 다음 문자열");
    ans = getchar();
    ans = tolower(ans);
    eatline();

    while(strchr("ulton", ans) == NULL) {
        puts("u, l, t, o, n 중에서 하나 선택: ");
        ans = tolower(getchar());
        eatline();
    }

    return ans;
}

void eatline(void) {
    while(getchar() != '\n') {
        continue;
    }
}

void ToUpper(char * str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

void ToLower(char * str) {
    while(*str) {
        *str = tolower(*str);
        str++;
    }
}

void Transpose(char * str) {
    while (*str) {
        if(islower(*str)) {
            *str = toupper(*str);
        }
        else if(isupper(*str)) {
            *str = tolower(*str);
        }
        str++;
    }
}

void Dummy(char * str) {
    //문자열을 그대로 둔다
}

void show(void (* fp)(char *), char * str) {
    (*fp)(str);
    puts(str);
}
```

```
문자열 입력(끝내려면 빈 라인):
How to C?
메뉴에서 원하는 직업 선택:
u) 대문자 변환    l)소문자 변환
t) 대소문자 교차 변환 o)원본을 그대로
n) 다음 문자열
u
HOW TO C?
메뉴에서 원하는 직업 선택:
u) 대문자 변환    l)소문자 변환
t) 대소문자 교차 변환 o)원본을 그대로
n) 다음 문자열
l
how to c?
메뉴에서 원하는 직업 선택:
u) 대문자 변환    l)소문자 변환
t) 대소문자 교차 변환 o)원본을 그대로
n) 다음 문자열
n
문자열 입력(끝내려면 빈 라인):

종료
```

이와 같은 상황에서 `typedef`를 사용할 수도 있다.

```c
typedef void (*V_FP_CHARP)(char *);
void show(V_FP_CHARP fp, char *);
V_FP_CHARP pfun;
```

좀 더 나아가, 그와 같은 포인터의 배열을 선언하고 초기화할 수 있다.

```c
V_FP_CHARP arpf[4] = {ToUpper, ToLower, Transpose, Dummy};
```

사용자가 선택하는 메뉴에 따라서 0부터 3까지 리턴하도록 하는 `int`형 함수가 되도록 `showmenu()`를 수정하면, `switch` 부분을 다음과 같이 대체할 수 있다.

```c
index = showmenu();

while (index >= 0 && index <= 3) {
    strcpy(copy, line);
    show(arpf[index], copy);    //선택된 함수를 사용
    index = showmenu();
}
```

이처럼 함수들의 배열은 불가능하지만 함수 포인터들의 배열은 가능하다.

아래 표는 함수 이름의 용도들을 요약한다.

함수 이름의 용도|예시
:---:|:---:
프로토타입 선언에 사용된 함수 이름|`int comp(int x, int y);`
함수 호출에 사용된 함수 이름|`status = comp(q, r);`
함수 정의에 사용된 함수 이름|`int comp(int x, int y) { ... }`
대입문에 포인터로 사용된 함수 이름|`pfunct = comp;`
포인터 전달인자로 사용된 함수 이름|`slowsort(arr, n, comp);`