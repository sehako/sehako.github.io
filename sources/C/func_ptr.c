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
