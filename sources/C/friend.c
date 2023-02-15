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
    struct person man[2] = {
        {
            {"James", "Hallow"},
            "Ramen",
            "Trainer",
            50000
        }, 
        {
            {"Bit", "Coin"},
            "Sushi",
            "Trader",
            100000   
        }
    };

    struct person * ptt;
    ptt = &man[0];
    
    printf("주소 #1: %p #2: %p\n", &man[0], &man[1]);
    printf("포인터 #1: %p #2: %p\n", ptt, ptt + 1);
    printf("ptt -> income: %f, (*ptt).income: %f\n", ptt -> income, (*ptt).income);
    ptt++;
    printf("ptt -> food: %s, ptt -> handle.last: %s\n", ptt -> food, ptt -> handle.last);

    return 0;
}