#include <stdio.h>
#include <stdlib.h>

int find(int a, int** b, int size);

int main(int argc, char const *argv[])
{
    int** b;
    b = (int**)malloc(5 * sizeof(int *));
    for(int i = 0; i < 5; i++)
        b[i] = (int*)malloc(2 * sizeof(int));

    b[0][0] = 0;

    b[1][0] = 1;

    b[2][0] = 2;

    b[3][0] = 3;
    b[4][0] = 4;
    
    b[0][1] = 0;

    b[1][1] = 2;

    b[2][1] = 4;

    b[3][1] = 6;
    b[4][1] = 9;

    printf("%d", find(6, b, 5));
    return 0;
}

int find(int a, int** b, int size) {
    for(int i = 0; i < size; i++)
        if(b[i][1] == a) return i;

    return -1;
}