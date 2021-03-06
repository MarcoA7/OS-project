#ifndef _STUDENT_H_
#define _STUDENT_H_
#define MATRICOLA 0
#define VOTO_ADE 1
#define NOF_ELEMS 2
#define NOF_INVITES 3
#define MAX_REJECT 4

#define STUDENT_FIELDS 5


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
int max_grade(group myGroup, int my_score, int size);



#endif
