#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int* a;
    a = malloc(5 * sizeof(int));
    printf("%lu\n", sizeof(a)/sizeof(int));
    return 0;
}
