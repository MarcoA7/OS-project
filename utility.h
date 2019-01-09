#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdlib.h>
#include <time.h>
#include <math.h>

#define INVITE 2
#define ACCEPT 1
#define REFUSE 3
#define ROWS 100
#define COLS 100
#define POSIX(x) x
#define WAITING_EVERYONE close(p_pipe[0]);\
                close(p_pipe[1]);\
                close(c_pipe[1]);\
                read(c_pipe[0], &c, 1);\
                close(c_pipe[0])\

#define READY_SET_GO close(p_pipe[1]);\
    read(p_pipe[0], &c, 1);\
    close(c_pipe[0]);\
    alarm(SIM_TIME);\
    close(c_pipe[1])

#define SET_UP_SYNC_MECH char c = 0;\
    int p_pipe[2], c_pipe[2];\
    pipe(p_pipe);\
    pipe(c_pipe)

#define POST_SENDING_MECH char p = 0;\
    int first_pipe2[2], second_pipe2[2];\
    pipe(first_pipe2);\
    pipe(second_pipe2)

#define EVERYONE_SENT_THEIR_MESSAGES close(first_pipe2[0]);\
    close(first_pipe2[1]);\
    close(second_pipe2[1]);\
    read(second_pipe2[0], &p, 1);\
    close(second_pipe2[0])

#define MERGE close(first_pipe2[1]);\
    read(first_pipe2[0], &p, 1);\
    close(second_pipe2[0]);\
    close(second_pipe2[1])

#define SETUP_MESSAGE       message.whoAmI[0][MATRICOLA] = mySelf->matricola;\
                            message.whoAmI[0][VOTO_ADE] = mySelf->voto_AdE;\
                            message.whoAmI[0][NOF_ELEMS] = mySelf->nof_elems;\
                            message.whoAmI[0][NOF_INVITES] = mySelf->nof_invites;\
                            message.whoAmI[0][MAX_REJECT] = mySelf->max_reject

#define ADD_TO_GROUP(elem)   myGroup->array[elem]->matricola = message.whoAmI[0][MATRICOLA];\
                            myGroup->array[elem]->voto_AdE = message.whoAmI[0][VOTO_ADE];\
                            myGroup->array[elem]->nof_elems = message.whoAmI[0][NOF_ELEMS];\
                            myGroup->array[elem]->nof_invites = message.whoAmI[0][NOF_INVITES];\
                            myGroup->array[elem]->max_reject = message.whoAmI[0][MAX_REJECT]

typedef struct matf {
	float data[ROWS][COLS];
} matf;

typedef matf* MatrixF;

typedef struct mati {
	int data[ROWS][COLS];
} mati;

typedef mati* MatrixI;
int find(int data, int* array, int length);
int findInMatrix(int data, int** array, int length, int position);
int concatenate(int a, int b);
//int randomValue(int lower_bound, int upper_bound);
//int randomEvenValue(int lower_bound, int upper_bound);
#endif
