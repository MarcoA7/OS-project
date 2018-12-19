#include <stdio.h>
#include <stdlib.h>

struct my_data {
    int array[10];
};

int find(int data, int* array, int length);

int main(int argc, char const *argv[])
{
    struct my_data* ok;
    ok = malloc(sizeof(struct my_data));
    for(int i = 0; i < 10; i++)
        ok->array[i] =0;
    ok->array[4] = 15;
    printf("%d", find(15, ok->array, 10));
    return 0;
}
int find(int data, int* array, int length) {
    for(int i = 0; i < length; i++)
        if (array[i] == data) return i;

    return -1;
}
