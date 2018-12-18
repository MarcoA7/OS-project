#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <string.h>
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
sem_t *mutex;
pid_t *all_student;

struct msg_s { 
    long type; 
    struct student* whoAmI; 
} message; 

int randomValue(int seed, int lower_bound, int upper_bound);
void sim_alarm_handler(int signum);

int main(int argc, char const *argv[])
{
    SET_UP_SYNC_MECH;
    int status;
    int sum; //total of all students
    int msgid;
    pid_t student_id;
    FILE* config;
    struct student* mySelf;
    group myGroup;

    //allocating memory for the group;
    myGroup = malloc(sizeof(myGroup));
    
    //initialization of the semaphore
    mutex = sem_open("accessing the board", O_CREAT, 0644, 1);
    //sem_unlink("accessing the board");

    // it has to become a shared variable
    MatrixF publicBoard;

    /* changing the default
    * alarm signal handler */
    signal(SIGALRM, sim_alarm_handler);

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
    publicBoard = (MatrixF)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (publicBoard == NULL)
    {
        printf("error In mmap()\n");
        exit(1);
    }
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
                mySelf->matricola = randomValue(i, 11111, 900000);
                mySelf->voto_AdE = randomValue(i, 18, 30);
                mySelf->nof_invites = publicBoard->data[POS_NOF_INVITES][0];
                mySelf->max_reject = publicBoard->data[POS_MAX_REJECTS][0];
                sem_wait(mutex);
                //the semaphore turns red.
                /* critical section */
                if(publicBoard->data[0][1] > 0) {
                        mySelf->nof_elems = publicBoard->data[0][0];
                        publicBoard->data[0][1]--;
                    }
                    else if(publicBoard->data[1][1] > 0) {
                        mySelf->nof_elems = publicBoard->data[1][0];
                        publicBoard->data[1][1]--;
                    }
                    else {
                        mySelf->nof_elems = publicBoard->data[2][0];
                        publicBoard->data[2][1]--;
                    }
                    publicBoard->data[i][2] = mySelf->matricola;
                    publicBoard->data[i][3] = mySelf->nof_elems;
                //the semaphore turn green.
                sem_post(mutex);

                /* setting up an empty group */
                myGroup->array = malloc(mySelf->nof_elems * sizeof(*mySelf));
                //mySelf->myGroup = malloc(mySelf->nof_elems * (sizeof(int)));
                memset(myGroup->array, 0, mySelf->nof_elems * (sizeof(*mySelf)));

                fprintf(stderr, "ready %d grades are %d max invites are: %d max rejects are: %d my team will have: %d people. My teacher is %d\n",mySelf->matricola, mySelf->voto_AdE, mySelf->nof_invites, mySelf->max_reject, mySelf->nof_elems, getppid());
                WAITING_EVERYONE;
                /* I'm the first member of the group */
                myGroup->array[0] = mySelf;
                while(1) {
                    /* open or create the message queue
                    * as the key for the queue we will use the matricola
                    */
                    /* MAndo nof_invites partendo dalla mia posizione nell'elenco degli
                    * alunni, tenendo conto se sono pari o dispari
                    * es: io sono il numero 2 mnado dal numero 3 al (3+nof_invites) % POP_SIZE
                    * evitando di invitare me stessov*/
                    msgid = msgget(mySelf->matricola, IPC_CREAT);
                    /* busy waiting until someone writes me */ 
                    while(msgrcv(msgid, &message, sizeof(message), INVITE, 0) == -1 && errno == EAGAIN);
                    /* the first one to "message me"
                    * will be accepted since I don't want to score zero
                    */
                    sem_wait(mutex);
                    if(isGroupEmpty(myGroup)) {
                        myGroup->array[1] = message.whoAmI;
                        //mySelf->myGroup[0] = message.whoAmI.matricola; 
                        msgid = msgget(message.whoAmI->matricola, IPC_CREAT);
                        message.whoAmI->matricola = mySelf->matricola;
                        message.type = ACCEPT;
                        msgsnd(msgid, &message, sizeof(message), 0);
                    }
                    else if(isGroupFull(myGroup, mySelf->nof_elems)) {
                        myGroup->closed = 1;
                        msgid = msgget(mySelf->matricola, IPC_CREAT);
                        msgctl(msgid, IPC_RMID, NULL);
                        exit(0); 
                    }
                    else {

                    }
                        
                    sem_post(mutex);

                }

                exit(0);
                break;
            default:
                /* teacher init*/
                all_student[i] = student_id;
                /* It will place all the students in the same process group (of the first student) */
                setpgid(all_student[i], all_student[0]); 
                break;
        }
    }
    /* waiting other student to end the creation */
    READY_SET_GO;
    /* OK everyone is ready time is starting... NOW */
    /* Start simulation */
    printf("Start looking for collegues\n");
    //alarm(SIM_TIME); // da chiedere
    int corpse;
    //int status;
    while ((corpse = wait(&status)) > 0)
    printf("%d: child %d exited with status 0x%.4X\n", (int)getpid(), corpse, status);
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
   //shmctl(m_id, IPC_RMID, NULL);

   //shmctl(m_id, IPC_RMID, NULL);
    return 0;
}
int randomValue(int seed, int lower_bound, int upper_bound) {
    srand(time(NULL) - seed * 2);
    int r = rand() % (upper_bound - lower_bound + 1) + lower_bound;
    return r;
}

void sim_alarm_handler(int signum) {
    /* it will terminate all the process in the same group as the first student */
  kill(-all_student[0], SIGINT);
}