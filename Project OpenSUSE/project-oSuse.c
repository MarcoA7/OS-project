/* COMPILE AS
* gcc -o project-oSuse project-oSuse.c -lm -pthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>

#define MAT_START 0
#define MAT_END 900000


#define ROWS_IN_FILE 5
#define NUMBER_OF_COMPOSITION 3
#define POS_NOF_INVITES 3
#define POS_MAX_REJECTS 4
/* the number of rows and cols of the shared memory */
#define ROWS 100
#define COLS 100
/* SHARED MEMORY */

#define POP_SIZE 6

#define SIM_TIME 3

sem_t mutex;

struct student {
    int matricola;
    int voto_AdE;
    int nof_elems;
    int nof_invites;
    int max_reject;
};

typedef struct matf {
	float data[ROWS][COLS];
} matf;

typedef matf* MatrixF;

struct msg_s { 
    long type; 
    struct student whoAmI; 
} message;

int randomValue(int seed, int lower_bound, int upper_bound);

int main(int argc, char const *argv[])
{
    sem_init(&mutex, 0, 1); 
    int sum; /* the sum of all students */
    pid_t *all_student, student_id;
    FILE* config; /* config file */
    struct student* mySelf; /* how is caratterzed a student */
    int m_id; /* shared memory identifier */
    MatrixF publicBoard; /* a public board where everyove can put everything */

    /* making the board public among every other process */
    m_id = shmget(IPC_PRIVATE, sizeof(*publicBoard), 0600);
    publicBoard = shmat(m_id, NULL, 0);

    /* loading the file into a data structer */
    /*****************************************/
    config = fopen("opt.conf", "r"); /* opening the file */
    for(int i = 0; i < NUMBER_OF_COMPOSITION; i++){
        fscanf(config, "%f %f", &publicBoard->data[i][0], &publicBoard->data[i][1]); /* retriving the number of elements and the percentage */
        /************************************************************
        * converting the percentage to numbers therefore            *
        * I am able to know the number of students for each group   *
        ************************************************************/
        publicBoard->data[i][1] = round(POP_SIZE * (float)publicBoard->data[i][1] / 100);
        if (i < NUMBER_OF_COMPOSITION - 1) sum += (int)publicBoard->data[i][1];
        else sum += sum - (int)publicBoard->data[i][1];
    }
    fscanf(config, "%f", &publicBoard->data[POS_NOF_INVITES][0]);
    fscanf(config, "%f", &publicBoard->data[POS_MAX_REJECTS][0]);

    for(int i = 0; i < 3; i++)
            printf("%.0f %.0f\n", publicBoard->data[i][0], publicBoard->data[i][1]);

    all_student = malloc( POP_SIZE * sizeof(all_student));

    /* Introduction */
    int j = 0;
    for(int i = 0; i < POP_SIZE; i++) {
        switch((student_id = fork())) {
            case 0:
                j=1;
                    /* student init */
                mySelf = malloc(sizeof(*mySelf));
                /*setting a random value for the matricola
                * since the time is the seed we need to scramble a bit more */
                mySelf->matricola = randomValue(i, 0, 900000);
                mySelf->voto_AdE = randomValue(i, 18, 30);
                mySelf->nof_invites = publicBoard->data[POS_NOF_INVITES][0];
                mySelf->max_reject = publicBoard->data[POS_MAX_REJECTS][0];
                sem_wait(&mutex);
                //the semaphore turns red.
                /* critical section */
                #if 0
                for(int i = 0; i < NUMBER_OF_COMPOSITION; i++)
                    if(publicBoard->data[i][1] > 0) {
                        mySelf->nof_elems = publicBoard->data[i][0];
                        publicBoard->data[i][1]--;
                    }
                #else
                    if(publicBoard->data[0][1] > 0) {
                        mySelf->nof_elems = publicBoard->data[0][0];
                        publicBoard->data[0][1]--;
                    }
                    else if(publicBoard->data[1][1] > 0) {
                        mySelf->nof_elems = publicBoard->data[1][0];
                        publicBoard->data[1][1]--;
                    }
                    else {
                        mySelf->nof_elems = publicBoard->data[1][0];
                        publicBoard->data[1][1]--;
                    }
                    for(int k = 0; k < 3; k++)
                        printf("%.0f %.0f\n", publicBoard->data[k][0], publicBoard->data[k][1]);
                #endif
                //the semaphore turn green.
                sem_post(&mutex);
                i = POP_SIZE;
                break;
            default:
                /* teacher init*/
                all_student[i] = student_id;
                break;
        }
    }
    if(j)
    printf("%d %d %d %d %d\n", mySelf->matricola, mySelf->voto_AdE, mySelf->nof_invites, mySelf->max_reject, mySelf->nof_elems);

    sem_destroy(&mutex); 
    return 0;
}


int randomValue(int seed, int lower_bound, int upper_bound) {
    srand(time(NULL) - seed * 2);
    int r = rand() % (upper_bound - lower_bound + 1) + lower_bound;
    return r;
}