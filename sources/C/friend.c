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