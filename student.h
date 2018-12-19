#ifndef _STUDENT_H_
#define _STUDENT_H_


struct student {
    int matricola;
    int voto_AdE;
    int nof_elems;
    int nof_invites;
    int max_reject;
};

typedef struct grp {
    struct student** array;
    int closed;
} grp;

typedef grp* group;

int isGroupEmpty(group theGroup);
int isGroupFull(group theGroup, int length);
int max_grade(group myGroup, int size);



#endif
