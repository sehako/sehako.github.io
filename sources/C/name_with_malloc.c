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