#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
struct my_struc {
    int a;
};
int main(int argc, char const *argv[])
{
    int j;
    struct my_struc *st;
    for(int i = 0; i < 3; i++)
    switch(fork()) {
        case 0:
            st = malloc(sizeof(*st));
            st->a = 15+i;
            j = getpid();
            break;
        default:
            j = getpid();
            break;
    }
    printf("my j is %d and st is: %d\n", j, st->a);
    return 0;
}
