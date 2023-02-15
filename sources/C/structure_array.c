#include <stdio.h>

#define FUNDLEN 50
#define N 2

struct funds {
    char bank[FUNDLEN];
    double bankfund;
    char save[FUNDLEN];
    double savefund;
};

double sum(const struct funds money[], int n);

int main(void) {
    struct funds jones[N] = {{
        "KB",
        40000.00,
        "ABC",
        4054.44
    },
    {
        "SH",
        20000.00,
        "EDF",
        30000.00
    }
    };

    printf("%f", sum(jones, N));

    return 0;
}

double sum(const struct funds money[], int n) {
    double total;
    int i;

    for(i = 0, total = 0; i < n; i++) {
        total += money[i].bankfund + money[i].savefund;
    }

    return total;
}