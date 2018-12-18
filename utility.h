#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdlib.h>
#include <time.h>

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

typedef struct matf {
	float data[ROWS][COLS];
} matf;

typedef matf* MatrixF;

typedef struct mati {
	int data[ROWS][COLS];
} mati;

typedef mati* MatrixI;

//int randomValue(int lower_bound, int upper_bound);
//int randomEvenValue(int lower_bound, int upper_bound);
#endif
