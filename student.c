#include "student.h"

int isGroupEmpty(group theGroup) {
    return theGroup->array[1]->matricola == 0 ? 1 : 0;
}

int isGroupFull(group theGroup, int length) {
    return theGroup->array[length - 1]->matricola == 0 ? 0 : 1;
}


int max_grade(group myGroup, int size) {
    int max = 0;
    for(int i = 0; i < size; i++)
        if(max < myGroup->array[i]->voto_AdE) max = myGroup->array[i]->voto_AdE;
    return max;
}