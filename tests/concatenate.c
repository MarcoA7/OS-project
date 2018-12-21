#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int concatenate(int a, int b);

int main(int argc, char const *argv[])
{
    int a = 0;
    int b = 55;
    printf("%d\n", concatenate(a, b));
    return 0;
}

int concatenate(int a, int b) {
    return a * pow(10, ceil(log10(b))) + b;
}