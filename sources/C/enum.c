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