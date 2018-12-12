#include "utility.h"
/*
int randomValue(int lower_bound, int upper_bound) {
    srand(time(NULL));
    int r = rand() % (upper_bound - lower_bound + 1) + lower_bound;
    return r;
}*/

/*
int randomEvenValue(int lower_bound, int upper_bound) {
    int r = randomValue(lower_bound, upper_bound);
    if(r % 2) return r;
    else return randomEvenValue(lower_bound, upper_bound);
}*/