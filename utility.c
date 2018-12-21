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
int find(int data, int* array, int length) {
    for(int i = 0; i < length; i++)
        if (array[i] == data) return i;

    return -1;
}
int findInMatrix(int data, int** array, int length, int position) {
    for(int i = 0; i < length; i++)
        if (array[i][position] == data) return i;

    return -1;
}

int concatenate(int a, int b) {
    return a * pow(10, ceil(log10(b))) + b;
}