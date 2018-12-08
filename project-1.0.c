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
#include <sys/msg.h>
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

struct msg_s { 
    long type; 
    struct student whoAmI; 
} message; 

int main(int argc, char const *argv[])
{
    int status;
    int sum; //total of all students
    int msgid;
    pid_t *all_student, student_id;
    FILE* config;
    struct student* mySelf;
    
    // it has to become a shared variable
    MatrixF publicBoard;

    #if POSIX(0)
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
    publicBoard = (MatrixF)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (publicBoard == NULL)
    {
        printf("error In mmap()\n");
        exit(1);
    }
    printf("TEST\n");
    #else
    int m_id = shmget(IPC_PRIVATE, sizeof(*publicBoard), 0600);
    publicBoard = shmat(m_id, NULL, 0);
    #endif

    /* loading the file into a data structer */
    /*****************************************/
    config = fopen("opt.conf", "r");
    for(int i = 0; i < NUMBER_OF_COMPOSITION; i++){
        fscanf(config, "%f %f", &publicBoard->data[i][0], &publicBoard->data[i][1]);
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
    all_student = malloc( POP_SIZE * sizeof(all_student));

    /* Introduction */

    for(int i = 0; i < POP_SIZE; i++) {
        switch((student_id = fork())) {
            case 0:
                    /* student init */
                mySelf = malloc(sizeof(*mySelf));
                mySelf->matricola = randomValue(0, 900000);
                mySelf->voto_AdE = randomValue(18, 30);
                mySelf->nof_invites = publicBoard->data[POS_NOF_INVITES][0];
                mySelf->max_reject = publicBoard->data[POS_MAX_REJECTS][0];
                sem_wait(&mutex);
                //the semaphore turns red.
                /* critical section */
                for(int i = 0; i < NUMBER_OF_COMPOSITION; i++)
                    if(publicBoard->data[i][1] > 0) {
                        mySelf->nof_elems = publicBoard->data[i][0];
                        publicBoard->data[i][1]--;
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
    /* waiting other student to end the creation */

    
    /* Start simulation */
    printf("Start looking for collegues\n");
    alarm(SIM_TIME);
    /*
    msgid = msgget(mySelf->matricola, 0666 | IPC_CREAT); 
    message.mesg_type = 1;

    message.whoAmI.matricola = mySelf->matricola;
    message.whoAmI.voto_AdE = mySelf->voto_AdE;

    for(int i = 0; i < mySelf->nof_invites; i++) {
        msgsnd(msgid, &message, sizeof(message), 0); 
        mySelf->nof_invites--;
    }
    while(1) {
         msgrcv(msgid, &message, sizeof(message), 1, 0);
    }
    */
   shmctl(m_id, IPC_RMID, NULL);
   printf("my mat is %d my grade is: %d my nof_ele is: %d my max ref is: %d my nof_invites is: %d\n", mySelf->matricola,
   mySelf->voto_AdE, mySelf->nof_elems, mySelf->max_reject, mySelf->nof_invites);

   shmctl(m_id, IPC_RMID, NULL);
    return 0;
}
