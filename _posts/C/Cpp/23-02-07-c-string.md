---
title:  "[C] 문자열과 문자열 입출력"
excerpt: " "

categories:
  - C

toc: true
toc_sticky: true
 
date: 2023-02-07
---

# 문자열 배열과 문자열 포인터

문자열을 선언할 떄, 배열 형식(`arr`)은 문자와 `null`('\0') 형식으로 메모리에 할당한다. `" "`로 둘러싸인 문자열은 실행 파일의 일부인 데이터 세그먼트에 저장되고, 프로그램이 메모리에 적재될 떄 그 문자열도 함께 적재된다. 이때 문자열은 정적 메모리(static memory)에 적재되어 있다고 하지만, 배열을 위한 메모리 할당은 프로그램의 실행이 시작된 후에 이루어진다. 그 시점에 문자열이 그 배열에 복사된다. 

그떄부터 컴파일러는, `arr`이 첫 번째 배열 원소의 주소 `&arr[0]`과 동일하다고 인식한다. 그리고 배열 형식에 사용된 `arr`이 주소상수(address constant)라는 점이다. 따라서 `arr`이라는 문자열 배열이 존재할 떄, `arr + 1`과 같은 연산은 허용되지만, `++arr`같은 연산은 허용되지 않는 것이다.

포인터 형식(`ptr`)또한 문자열을 정적 메모리에 저장한다. 또한 프로그램이 실행을 시작하면, 포인터 변수를 위한 기억 위치를 하나 더 할당받고, 그 변수에 문자열이 담긴 정적 메모리의 주소를 저장한다. `ptr`은 문자열의 첫 번째 원소를 가리키고, 또한 그것을 변경할 수 있다. 그러므로 `++ptr`같은 연산이 가능하고 해당 연산은 두 번째 문자를 가리키게 된다.

문자열 리터럴은 `const` 데이터로 간주된다. `ptr`이 데이터를 가리키기 때문에 `const` 데이터를 가리키고 있다고 선언되어야 한다. 이것은 `ptr`의 값을 변경할 수 없다는 것이 아니라, `ptr`을 사용해 데이터 자체의 값을 바꿀 수 없다는 뜻이다. 반면에, 만약 사용자가 배열에 문자열 리터럴을 복사한다면 `const`로 배열을 선언하지 않더라도 데이터를 마음대로 변경할 수 있다.

배열의 초기화는 정적 메모리에서 배열로, 포인터의 초기화는 문자열의 주소만을 복사한다. 아래 코드는 그에 대한 예제이다.

```c
#define MSG "I'm special."

#include <stdio.h>

int main(void) {
    char ar[] = MSG;
    const char *pt = MSG;
    
    printf("I'm special.의 주소값: %p\n", "I'm special.");
    printf("ar: %p\n", ar);
    printf("pt: %p\n", pt);
    printf("MSG: %p\n", MSG);
    printf("I'm special.의 주소값: %p\n", "I'm special.");

    return 0;
}
```

```
I'm special.의 주소값: 00007FF732945050
ar: 000000395CFFFBBB
pt: 00007FF732945050
MSG: 00007FF732945050
I'm special.의 주소값: 00007FF732945050
```

## 배열과 포인터의 차이

포인터가 유일하게 증감 연산자를 사용할 수 있다는 것 외에도, 배열은 배열이 담고 있는 문자열을 변경할 수 있는 방법이 존재한다.

```c
arr[3] = 'A'
```

포인터를 사용하여 문자열을 변경한다면 메모리 접근 에러를 일으킬 수 있다. 동일한 문자열 리터럴들을 메모리에 유일본으로 나타내는 것을 컴파일러가 선택할 수 있기 때문이다. 

```c
char * pt = "Hello";
pt[0] = 'F';
printf("Hello");
printf("%s", "Hello");
```

다음과 같은 코드가 존재한다면, 모든 문장들이 문자열 `Hello`의 유일한 메모리 위치를 참조할 수 있다. 컴파일러에 따라 다르지만 저런 상황에서 어떤 컴파일러는 `Fello`라고 출력할 것이고 어떤 컴파일러는 실행을 중단할 것이다. 그러므로 포인터를 문자열 리터럴로 초기화할 때, `const` 변경자를 사용하는 것이 바람직하다.

```c
const char * pt = "Hello";  //권장 방식
```

그러나 배열을 문자열 리터럴로 초기화하는 것은 문제를 일으키지 않는다. 왜냐면 그 배열이 원본 문자열의 **복사본**을 얻게 되기 떄문이다. 따라서 문자열을 변경하고자 한다면, 문자열 리터럴에는 배열이 추천된다.

# 문자열들의 배열

문자열들의 배열을 사용하면 하나의 인덱스만 사용하여 서로 다른 여러 문자열에 접근할 수 있다.

```c
#include <stdio.h>
#define SLEN 20
#define LIM 5

int main(void) {
    const char *arr[LIM] = {
        "Hello",
        "World!",
        "This",
        "is",
        "C language"
    };

    char ar[LIM][SLEN] = {
        "String",
        "and",
        "Pointer",
        "very",
        "interesting!"
    };

    for(int i = 0; i < LIM; i++) {
        printf("%-36s%-25s\n", arr[i], ar[i]);
    }
    printf("\nsizeof arr: %zd, sizeof ar: %zd\n", sizeof(arr), sizeof(ar));

    return 0;
}
```

```
Hello                               String
World!                              and
This                                Pointer
is                                  very
C language                          interesting!

sizeof arr: 40, sizeof ar: 100
```

`arr` 배열은 시스템의 40 바이트까지 차지하는 5개의 포인터 배열이다. 그러나 `ar`은 각 `20 char` 값인 배열 5개의 배열이고 시스템상에서 100 바이트를 차지한다. 따라서 두 변수가 모두 문자열에 관한 것일지라도 엄연히 다른 타입이다. `arr`은 초기화에 사용된 문자열 리터럴의 위치를 가리키는데, 이것은 정적인 메모리에 저장된다. 그러나 `ar`내의 배열은 문자열 리터럴의 복사본이 포함되었기에 각 문자열이 두 번씩 저장된다.

또한 배열 내 메모리 할당이 비효율적이다. 그 이유는 배열 내 모든 크기는 고정되어 있으며, 그 크기는 항상 가장 긴 문자열을 유지할 정도로 충분해야 하기 때문이다.

따라서 문자열 한 묶음을 나타내는 배열을 사용하기를 원한다면 포인터의 배열은 문자 배열들의 배열보다 더 효율적이다. 그러나, 포인터들은 문자열 리터럴을 가리키기 때문에 이 문자열은 수정이 불가능하다. 하지만 배열은 수정이 가능하다. 따라서 문자열을 고치거나 문자열 입력을 위한 공간을 확보하고자 한다면 포인터 대신 배열을 사용하는 것이 좋을 것이다. 


## 포인터와 문자열

문자열에 대한 대부분의 연산은 포인터를 통하여 이루어진다.

```c
#include <stdio.h>

int main(void) {
    const char * msg = "This is msg";
    const char * copy;

    copy = msg;

    printf("%s\n", copy);
    printf("msg = %s    &msg = %p   값 = %p\n", msg, &msg, msg);
    printf("copy = %s   &copy = %p  값 = %p\n", copy, &copy, copy);

    return 0;
}
```

```
This is msg
msg = This is msg    &msg = 000000CFBD1FFAC8   값 = 00007FF77A0C5050
copy = This is msg   &copy = 000000CFBD1FFAC0  값 = 00007FF77A0C5050
```

두 포인터 모두 같은 주소를 가리키고 있다는 것을 확인할 수 있다. 다시 말해서, 문자열 자체는 결코 복사되지 않았다. `copy = msg;`는 같은 문자열을 가리키는 제 2의 포인터를 만든 것이다.

# 문자열 입력

## 기억 공간 만들기

읽을 문자열을 충분히 저장할 수 있는 만큼의 기억 공간을 할당할 필요가 있다. 가장 쉬운 방법은, 선언에 배열 크기를 명시적으로 지정하는 것이다.

```c
char var[SIZE];
```

이러면 `var` 변수는 `SIZE` 바이트 만큼 할당된 메모리 블록의 주소가 할당될 것이다. 

다른 방법은 메모리를 할당하는 C 라이브러리 함수를 사용하는 것이다. (추후 링크 기술)

## gets() 함수 

`scanf()`와 `%s` 지정자는 한 단어만 읽는다는 것을 기억해야 한다. 한 번에 한 단어 대신 입력 줄 전체를 읽을 수 있는 함수가 바로 `gets()`이다. `gets()`는 개행문자(`\n`)에 도달할 때까지 한 줄을 전부 읽고, 개행문자를 제거하고 null을 추가하여 저장한다. 이에 관한 예제이다.

```c
#include <stdio.h>
#define STLEN 81

int main(void) {
    char words[STLEN];

    puts("Input String: ");
    gets(words);
    printf("-----------\n");
    printf("%s\n", words);
    puts(words);
    puts("END.");

    return 0;
}
```

```
Input String:
HELLO   //입력값
-----------
HELLO
HELLO
END.
```

`puts(words);`는 `printf("%s\n", words);`와 같은 효과가 있다.

하지만 `gets()`는 입력행이 실제로 배열에 딱 맞는지 볼 수 있도록 점검하지 않는다는 것이다. `gets()`는 배열이 어디서 시작하는지만 알 뿐 원소가 몇 개 있는지는 모른다. 입력 문자열이 너무 길면, 버퍼 오버플로(buffer overflow)가 나타나는데, 이는 지정된 목표를 초과하는 문자들이 넘친다는 의미이다. 이런 문제 때문에 `gets()`는 표준에서 제외되었다. 물론 대부분의 컴파일러는 백워드 호환성에 대한 이해관계에서 `gets()`를 계속 지원할 것이다.

## gets()에 대한 대안들

전형적으로 `fgets()`가 `gets()`의 대안으로 존재한다. `fgets()`는 인터페이스가 좀 더 복잡하고 입력을 약간 다르게 처리한다. 따라서 `gets_s()`를 표준으로 추가하기도 하였다. 

### fgets() & fputs()

`fgets()`는 읽을 문자들의 최대 개수를 지정함으로써 두 번째 전달인자를 취할 때 생길 수 있는 오버플로 문제를 해결한다. 해당 함수는 파일 입출력용으로 설계된 것이기 때문에 `gets()`와 세 가지 측면에서 차이점이 존재한다.

- 읽을 문자들의 최대 개수를 지정하기 위해 두 번째 전달인자로 사용한다. n을 전달인자로 받으면 n - 1개까지 문자들을 읽거나 개행 문자가 나올 때까지 읽는다.

- `fgets()`는 개행 문자를 읽어 그 문자열에 저장한다. `gets()`는 읽은 다음 버린다.

- 읽을 파일을 지정하는 세 번째 전달인자를 사용한다. 기보드로부터 읽으려면 `stdin`을 전달인자로 사용하고 이 식별자는 `stdio.h`에 정의되어 있다.

`fgets()`는 `fputs()`와 짝을 이루어 주로 사용된다. `fputs()`는 개행을 추가하지 않고 파일 입출력에 관련된 함수이기 떄문에 마찬가지로 `stdio.h`에 정의된 식별자인 `stdout`을 전달인자로 사용하면 된다. 

```c
#include <stdio.h>
#define STLEN 8

int main(void) {
    char words[STLEN];

    puts("Input String: ");
    fgets(words, STLEN, stdin);
    printf("------------------\n");
    puts(words);
    fputs(words, stdout);

    puts("Input String: ");
    fgets(words, STLEN, stdin);
    printf("------------------\n");
    puts(words);
    fputs(words, stdout);
    puts("END.");

    return 0;
}
```

```
Input String:
HELLO
------------------
HELLO

HELLO
Input String:
HELLO WORLD!
------------------
HELLO W
HELLO WEND.
```

첫 번째 입력은 `HELLO\n\0`을 저장할 수 있다. 하지만 두 번쨰 입력은 그렇지 못한다. 배열이 담을 수 있는 크기의 한계를 초과하므로 `fgets()`는 `\n`을 제거하고 `HELLO W\0`을 저장한다. 따라서 `\n`을 자동으로 추가하는 `puts()`와 다르게, `fputs()`는 개행을 하지 않는다.

`fgets()` 함수는 `char`로 포인터를 리턴한다. 문제가 없다면, 그것을 첫 번째 전달 인자로서 보내졌던 것과 같은 주소로 리턴한다. 그러나 함수가 파일끝을 만나면 널 포인터(null pointer)를 리턴한다. 이는 특별한 경우를 가리키는데 사용될 수 있도록 유효한 데이터를 가리키지 않도록 보증된 포인터이다. 널 포인터는 숫자 0 또는 범용적으로 `macro NULL`로 표현된다. 이는 함수의 읽기 에러 발생 시 리턴되는 값이다.

이를 이용하여 입력받은 값을 즉각적으로 에코하는 간단한 프로그램을 구성할 수도 있다.

```c
#include <stdio.h>
#define STLEN 10

int main(void) {
    char words[STLEN];
    
    puts("Input String: ");
    while (fgets(words, STLEN, stdin) != NULL && words[0] != '\n') {
        fputs(words, stdout);
    }
    puts("END.");

    return 0;
}
```

```
Input String:
HELLO
HELLO
WORLD!
WORLD!
HELLO WORLD!
HELLO WORLD!

END.
```

정의된 `STLEN` 상수가 10이지만, 프로그램은 더 긴 입력행 또한 처리하는데 문제가 없다. `HELLO WOR\0`을 읽고 저장한 후, `fputs()`로 출력한다. 그 다음 다시 `LD\n\0`를 읽고 저장한 다음 출력하는 식이다.

시스템은 버퍼를 사용하는 I/O(buffered I/O)를 사용한다. 입력이 리턴키가 눌러질 때까지 임시 메모리(버퍼)에 저장된다. 이것은 입력에 `\n`을 추가하여 전체 행을 `fgets()`상으로 보낸다. 개행이 보내졌을 때 버퍼의 내용은 화면상으로 보내진다. 

`\n`을 제거하는 방법도 존재한다. 한 가지 방법은 저장된 개행용 문자열을 찾고 `NULL` 문자로 대체하는 것이다.

```c
#include <stdio.h>
#define STLEN 10

int main(void) {
    char words[STLEN];
    int i;

    puts("Input String: ");

    while (fgets(words, STLEN, stdin) != NULL && words[0] != '\n') {
        i = 0;
        
        while (words[i] != '\n' && words[i] != '\0')
        {
            i++;
        }

        if (words[i] == '\n') {
            words[i] = '\0';
        }
        else {
            while (getchar() != '\n') {
                continue;
            }
        }
        puts(words);
    }
    puts("END.");

    return 0;
}
```

```
Input String:
HELLO
HELLO
HELLO WORLD!
HELLO WOR

END.
```

**널 문자와 널 포인터**

    널 문자와 널 포인터는 개념적으로 서로 다르다. 널 문자(\0)는 문자열의 끝을 표시하는데 사용된다. 
    널 포인터(NULL)는 데이터의 유효한 주소에 해당하지 않는 값을 갖는다. 파일 끝을 만나거나 실행하는데 실패하는 것과 같은 일부 특별한 발생을 나타내기 위해 유효 주소를 반환하는 함수에서 사용된다.
    따라서 널 문자는 정수형(1 바이트)이고 널 포인터는 포인터 형(일반적으로 4 바이트)이다. 

### gets_s()

선택적인 `gets_s()` 함수는 전달인자가 읽는 문자 수의 한계를 사용한다. 

```c
gets_s(words, STLEN);
```

`fgets()` 함수는 다음과 같이 세 가지의 주된 차이점이 있다.

- 표준 입력만 읽기에 두 개의 전달인자가 필요하다
- `\n`을 읽는 경우 저장하는 대신 버린다
- 문자를 최대치까지 읽고 `\n`을 읽는데 실패했다면, 몇 단계를 거친다. 목표 배열의 첫 번째 문자를 널 문자에 맞추어 놓는다. 개행 또는 파일의 끝(EOF-End Of File)을 만날 때까지 그것을 읽고 이후의 입력을 버린다. 그 후 널 포인터(NULL)를 리턴한다. 구현에 의존하는 "handler" 함수를 호출하는데(또 다른 하나 사용자가 선택한), 이것이 프로그램의 종료나 중단을 야기할 수 있다.

`gets()`, `fgets()`, `gets_s()`를 비교하면 세 함수 모두 입력 범위(저장공간)에 맞다면 문제가 없다. 그러나 `fgets()`가 문자열의 일부로 개행을 포함시키기 떄문에 널 문자로 그것을 대체하는 코드를 제공할 필요가 있다.

입력행이 확보한 기억공간에 맞지 않다면 `gets()` 데이터를 손상시키고 보안을 위태롭게 할 수 있다. `gets_s()`는 프로그램이 중단이나 종료되도록 하지 않으려면 특별한 handler를 작성하고 등록하는 법을 배울 필요가 있다. 결론적으로 `fgets()` 함수가 위 문제에 대한 가장 안전한 방법으로 볼 수 있다.

# 문자열 출력

## puts()

`puts()`는 문자열의 주소를 전달인자로 전달하기만 하면 된다. 

```c
#include <stdio.h>
#define DEF "This is #define"

int main(void) {
    char str[80] = "This is String";
    const char * strg = "This is pointer String";

    puts(DEF);
    puts(str);
    puts(strg);
    puts(&str[5]);
    puts(strg + 4);

    return 0;
}
```

```
This is #define
This is String
This is pointer String
is String
 is pointer String
```

`puts()`는 지정된 시작한 지점에서 문자열의 끝(널 문자)까지 출력한다. 그러므로 널 문자가 없다면, `puts()`는 어디에서 멈추어야 하는지 알지 못한다. 

## fputs()

`fputs()`는 파일 관련 출력 함수이기 때문에, 출력 전달인자를 지정해야 하고 출력에 개행을 자동으로 추가하지 않는다는 차이점이 있다. 선언 형식은 다음과 같다.

```c
fputs(words, stdout);
```

# 문자열 함수
C 라이브러리는 몇 개의 문자열 처리 함수를 제공하고 그런 프로토타입을 `string.h` 헤더 파일을 사용한다. 

## strlen()
문자열의 길이를 출력한다. 선언 방법은 문자열 `string` 변수가 존재한다면 다음과 같다.

```c
strlen(string);
```

## strcat()
문자열 결합을 수행하는 함수이다. 두 개의 문자열을 전달인자로 사용하므로 두 개의 문자열 `str`과 `strg`가 있다면 선언 방법은 다음과 같다.

```c
strcat(str, strg);
```

`str` 변수에 `strg`의 내용이 추기된다.

## strncat()
`strcat()`은 두 번째 문자열이 첫 번째 배열에 맞는지 검사하지 않는다. 따라서 오버플로우의 위험성이 존재한다. 따라서 추가할 문자 개수의 최대값을 지정하기 위해 두 번째 전달인자를 사용하는 `strncat()`을 사용할 수 있다. 

```c
strncat(str, strg, 10);
```

위 함수는 `strg`의 내용을 `str`에 추가할 때 문자 개수가 10개에 도달하거나 널 문자를 만나면 추가를 멈춘다. 

## strcmp()
문자열을 비교하는 함수이다. 

문자열을 단순히 숫자 비교하듯이 `!=`나 `==`로 비교할 수 없다. 예를 들어 입력받은 변수(`str`)와 기존의 변수(`strg`)를 비교하는 조건문을 만들고 싶다면

```c
if (str != strg)
```

이렇게 쓸 수 있다고 생각하지만, 이는 작동하지 않는다. 두 변수는 실제로는 포인터이다. 따라서 위와같은 비교는 두 문자열이 같은지 검사하지 않는다. 그 대신 두 문자열의 주소가 같은지 검사한다. 하지만 두 주소는 서로 다르기에 내용에 상관없이 항상 해당 조건문이 실행될 것이다. 

주소가 아닌 내용을 비교하고자 할 때 바로 `strcmp()`이다. 이는 두 문자열 전달 인자가 같으면 0을, 같지 않으면 0이 아닌 숫자를 리턴한다. 선언 방법은 다음과 같다.

```c
strcmp(str, strg);
```
위 함수의 좋은 점은, 배열이 아닌 문자열을 비교하기 때문에 서로 다른 크기를 가진 배열에 저장되어 있는 문자열들을 비교하는 데에도 사용할 수 있다. 

대부분의 상황에서 0이 아닌 값을 리턴한다는 정도만 알면 사실 끝이지만, 문자열을 알파벳 순서로 정렬(sorting) 하고자 할 때, 리턴값이 양수인지, 0인지, 음수인지 알아야 한다. 

```c
strcmp("A", "B");   // -1
```

`strcmp()`는 첫 번째 문자열이 두 번쨰 문자열보다 알파벳 순서로 앞에 오면 음수를, 뒤에 오면 양수를 리턴한다고 정의한다. 그러나 이에 대한 정확한 수치값은 컴파일러에게 위임되어 있다. 

## strncmp()
`strcmp()`는 대응하는 두 문자가 일치하지 않는 문자 쌍을 찾을 떄까지 문자열들을 비교한다. 이것은 둘 중 한 문자열의 끝에 도달할 떄까지 검색이 이루어질 수 있다는 것을 의미한다. `strncmp()`는 지정한 문자들의 개수까지만 문자열들을 비교한다.

```c
strncmp(str, strg, 5);
```

## strcpy()

문자열의 주소가 아닌 문자열의 내용을 복사하고자 할 떄 사용하는 함수다.

```c
strcpy(target, source);   //source의 내용을 target에 복사한다.
```

`strcpy`는 두 가지 유용한 특성을 더 가지고 있다.

1. `char *`형이기 때문에 첫 번째 전달인자의 값인 문자의 주소를 리턴한다.
2. 첫 번째 전달인자가 반드시 배열의 시작을 가리킬 필요는 없다. 따라서 배열의 일부만 복사할 수도 있다.

```c
strcpy(target + n, source);
```
위와 같은 선언은 `target`의 (n + 1)번째 원소를 가리키고 (n + 1)번쨰 원소부터 `source`의 내용을 복사한다.

## strncpy()

`strcpy()`는 `gets()`와 같이 버퍼 오버플로의 문제를 안고있다. 따라서 `strncpy()`는 복사할 최대 문자 개수를 지정하는 전달인자를 추가하여 이런 문제를 해결했다.

```c
strncpy(target, source, n);
```
위와 같은 선언은 n개의 문자들까지 또는 널 문자를 만날 때까지 `source`의 문자열을 `target`에 복사한다. 만약 n 개의 문자보다 더 큰 경우 널 문자는 추가되지 않는다. 따라서 함수 사용 시 널 문자를 추가할 방법을 고려하는 것이 좋다.

## sprintf()
`stdio.h`에 선언되어 있는 함수로 `printf()`와 비슷하지만 화면이 아닌 문자열에 출력한다는 차이점이 있다. 따라서 여러 항목들을 하나의 문자열로 결합하는 방법을 제공한다. 

```c
sprintf(target, "%s: %s\n", src, source);
```

다음 코드는 `target`이라는 배열에 `src`와 `source`의 내용을 `"%s: %s\n"`의 형태로 저장한다.

이 외에도 다양한 문자열 함수들이 존재한다. 