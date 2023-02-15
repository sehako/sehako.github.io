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