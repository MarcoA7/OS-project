#include "student.h"

int isGroupEmpty(group theGroup) {
    return theGroup->array[1]->matricola == 0 ? 1 : 0;
}

int isGroupFull(group theGroup, int length) {
    return theGroup->array[length - 1]->matricola == 0 ? 0 : 1;
}