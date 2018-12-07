#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdlib.h>
#include <time.h>

#define ROWS 100
#define COLS 100
#define POSIX(x) x

typedef struct matf {
	float data[ROWS][COLS];
} matf;

typedef matf* MatrixF;

typedef struct mati {
	int data[ROWS][COLS];
} mati;

typedef mati* MatrixI;

int randomValue(int lower_bound, int upper_bound);
int randomEvenValue(int lower_bound, int upper_bound);
#endif
