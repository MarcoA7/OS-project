#include <stdio.h>
#include <stdlib.h>

struct student {
    int matricola;
    int voto_AdE;
    int nof_elems;
    int nof_invites;
    int max_reject;
};

typedef struct grp {
    struct student* *array;
    int closed;
} grp;

typedef grp* group;

int main(int argc, char const *argv[])
{
    group myGroup;
    struct student* mySelf;

    myGroup = malloc(sizeof(myGroup));

    mySelf = malloc(sizeof(*mySelf));
    mySelf->matricola = 3;
    mySelf->max_reject = 9;
    mySelf->nof_elems = 0;
    mySelf->nof_invites = 8;
    mySelf->voto_AdE = 9;

    myGroup->array = malloc(2 * sizeof(*mySelf));
    myGroup->array[0] = mySelf;
    printf("%d", myGroup->array[0]->matricola);
    return 0;
}

