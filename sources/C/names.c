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