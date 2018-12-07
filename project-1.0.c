#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "student.h"
#include "utility.h"

#define ROWS_IN_FILE 5
#define NUMBER_OF_COMPOSITION 3
#define POS_NOF_INVITES 3
#define POS_MAX_REJECTS 4

#ifndef POP_SIZE
#define POP_SIZE 6
#endif

#ifndef SIM_TIME
#define SIM_TIME 3
#endif
sem_t mutex;

int main(int argc, char const *argv[])
{
    int status;
    int sum; //total of all students
    pid_t *all_student, student_id;
    FILE* config;
    student mySelf;
    
    // it has to become a shared variable
    MatrixF parameters;

    #if POSIX(1)
    int shm_fd;
	void *ptr;
    int shared_seg_size = (sizeof(MatrixF));
    shm_fd = shm_open("project", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if (shm_fd < 0)
    {
        printf("Error In shm_open()");
        exit(1);
    }

    printf("Created shared memory object %s\n", "project");

    /* adjusting mapped file size (make room for the whole segment to map)      --  ftruncate() */
    ftruncate(shm_fd, shared_seg_size);

    /* requesting the shared segment    --  mmap() */
    parameters = (MatrixF)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (parameters == NULL)
    {
        printf("error In mmap()\n");
        exit(1);
    }
    printf("TEST\n");
    #else
    int m_id = shmget(IPC_PRIVATE, sizeof(*parameters), 0600);
    parameters = shmat(m_id, NULL, 0);
    #endif

    /* loading the file into a data structer */
    /*****************************************/
    config = fopen("opt.conf", "r");
    for(int i = 0; i < NUMBER_OF_COMPOSITION; i++){
        fscanf(config, "%f %f", &parameters->data[i][0], &parameters->data[i][1]);
        /************************************************************
        * converting the percentage to numbers therefore            *
        * I am able to know the number of students for each group   *
        ************************************************************/
        parameters->data[i][1] = round(POP_SIZE * (float)parameters->data[i][1] / 100);
        if (i < NUMBER_OF_COMPOSITION - 1) sum += (int)parameters->data[i][1];
        else sum += sum - (int)parameters->data[i][1];
    }
    fscanf(config, "%f", &parameters->data[POS_NOF_INVITES][0]);
    fscanf(config, "%f", &parameters->data[POS_MAX_REJECTS][0]);
    all_student = malloc( POP_SIZE * sizeof(all_student));

    /* Introduction */

    for(int i = 0; i < POP_SIZE; i++) {
        switch((student_id = fork())) {
            case 0:
                    /* student init */
                mySelf = malloc(sizeof(*mySelf));
                mySelf->matricola = randomValue(0, 900000);
                mySelf->voto_AdE = randomValue(18, 30);
                mySelf->nof_invites = parameters->data[POS_NOF_INVITES][0];
                mySelf->max_reject = parameters->data[POS_MAX_REJECTS][0];
                sem_wait(&mutex);
                //the semaphore turns red.
                /* critical section */
                for(int i = 0; i < NUMBER_OF_COMPOSITION; i++)
                    if(parameters->data[i][1] > 0) {
                        mySelf->nof_elems = parameters->data[i][0];
                        parameters->data[i][1]--;
                    }
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

    
    /* Start simulation */
    printf("Start looking for collegues\n");
    alarm(SIM_TIME);

    

    return 0;
}
