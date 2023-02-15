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