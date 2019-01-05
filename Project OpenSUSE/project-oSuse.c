/* COMPILE AS
* gcc -o project-oSuse project-oSuse.c -lm -pthread
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>

#define WAITING_EVERYONE close(first_pipe[0]);\
                close(first_pipe[1]);\
                close(second_pipe[1]);\
                read(second_pipe[0], &c, 1);\
                close(second_pipe[0])\

#define READY_SET_GO close(first_pipe[1]);\
    read(first_pipe[0], &c, 1);\
    close(second_pipe[0]);\
    alarm(SIM_TIME);\
    close(second_pipe[1])

#define SET_UP_SYNC_MECH char c = 0;\
    int first_pipe[2], second_pipe[2];\
    pipe(first_pipe);\
    pipe(second_pipe)

#define ADD_TO_GROUP(elem)   myGroup->array[elem]->matricola = message.whoAmI[0][MATRICOLA];\
                            myGroup->array[elem]->voto_AdE = message.whoAmI[0][VOTO_ADE];\
                            myGroup->array[elem]->nof_elems = message.whoAmI[0][NOF_ELEMS];\
                            myGroup->array[elem]->nof_invites = message.whoAmI[0][NOF_INVITES];\
                            myGroup->array[elem]->max_reject = message.whoAmI[0][MAX_REJECT]

#define SETUP_MESSAGE       message.whoAmI[0][MATRICOLA] = mySelf->matricola;\
                            message.whoAmI[0][VOTO_ADE] = mySelf->voto_AdE;\
                            message.whoAmI[0][NOF_ELEMS] = mySelf->nof_elems;\
                            message.whoAmI[0][NOF_INVITES] = mySelf->nof_invites;\
                            message.whoAmI[0][MAX_REJECT] = mySelf->max_reject

#define MAT_START 0
#define MAT_END 900000

#define INVITE 2
#define ACCEPT 1
#define REFUSE 3
#define MATRICOLA 0
#define VOTO_ADE 1
#define NOF_ELEMS 2
#define NOF_INVITES 3
#define MAX_REJECT 4
#define ROWS_IN_FILE 5
#define NUMBER_OF_COMPOSITION 3
#define POS_NOF_INVITES 3
#define POS_MAX_REJECTS 4
#define STUDENT_FIELDS 5
/* the number of rows and cols of the shared memory */
#define ROWS 100
#define COLS 100
/* SHARED MEMORY */

#define POP_SIZE 6

#define SIM_TIME 10


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
    int whoAmI[1][STUDENT_FIELDS]; 
} message;

typedef struct grp {
    struct student** array;
    int closed;
} grp;

typedef grp* group;

pid_t all_student[POP_SIZE]; /* array of all students */
int randomValue(int seed, int lower_bound, int upper_bound);
int findInMatrix(int data, int** array, int length, int position);
int max_grade(group myGroup, int size);
void sim_alarm_handler(int signum);

int main(int argc, char const *argv[])
{   
    SET_UP_SYNC_MECH; /* Setting up sync mechanism */
    sem_t *mutex;/* POSIX semaphore */
    int sum; /* the sum of all students */
    int elem = 0; /* to identify the number of students in my group */
    pid_t student_id;
    FILE* config; /* config file */
    struct student* mySelf; /* how is caratterzed a student */
    struct student* firend; /* I have a friend */
    group myGroup; /* the group of the project */
    int m_id; /* shared memory identifier */
    int my_msgid, friend_id; /* my message queue id */
    MatrixF publicBoard; /* a public board where everyove can put everything */
    int myPosition; /* where I'm placed in the list of all sutdent */
    int **student_list; /* list of all student */

    student_list = (int**)malloc(POP_SIZE * sizeof(int*));
    for(int i = 0; i < POP_SIZE; i++)
        student_list[i] = (int*) malloc(2 * sizeof(int));
    
    /* allocating memory for the group */
    myGroup = malloc(sizeof(myGroup));
    
    /* changing the default
    * alarm signal handler */
    signal(SIGALRM, sim_alarm_handler);
    /* initialization of the semaphore */
    /* opening a shared semphore with a common name */
    mutex = sem_open("/nof_elem", O_CREAT,  0644, 1); /* a semaphore for a shared variable */

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
    fscanf(config, "%f", &publicBoard->data[POS_NOF_INVITES][0]); /* retriving the nof_invites */
    fscanf(config, "%f", &publicBoard->data[POS_MAX_REJECTS][0]); /* retriving the max _rejects */

    /* printing the board */

    for(int i = 0; i < 3; i++)
            printf("%.0f %.0f\n", publicBoard->data[i][0], publicBoard->data[i][1]);

    //all_student = malloc( POP_SIZE * sizeof(all_student));
    
    /* Introduction */
    for(int i = 0; i < POP_SIZE; i++) {
        switch((student_id = fork())) {
            case 0:
                    /* student init */
                mySelf = malloc(sizeof(*mySelf));
                firend = malloc(sizeof(*firend));
                /*setting a random value for the matricola
                * since the time is the seed we need to scramble a bit more */
                mySelf->matricola = randomValue(i, 0, 900000);
                mySelf->voto_AdE = randomValue(i, 18, 30);
                mySelf->nof_invites = publicBoard->data[POS_NOF_INVITES][0];
                mySelf->max_reject = publicBoard->data[POS_MAX_REJECTS][0];
                sem_wait(mutex);
                //the semaphore turns red.
                /*decresing the number of elements is a critical action
                * therefore I need a semaphore */
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
                        mySelf->nof_elems = publicBoard->data[2][0];
                        publicBoard->data[2][1]--;
                    }
                    publicBoard->data[i][2] = mySelf->matricola;
                    //publicBoard->data[i][3] = mySelf->nof_elems;
                    publicBoard->data[i][3] = mySelf->voto_AdE;
                #endif
                //the semaphore turn green.
                sem_post(mutex);
                /* setting up an empty group */
                myGroup->array = malloc(mySelf->nof_elems * sizeof(*mySelf));
                //mySelf->myGroup = malloc(mySelf->nof_elems * (sizeof(int)));
                memset(myGroup->array, 0, mySelf->nof_elems * (sizeof(*mySelf)));
                myGroup->closed = 0;

                fprintf(stderr, "I'm %d ready %d grades are %d max invites are: %d max rejects are: %d my team will have: %d people. My teacher is %d\n", getpid(),mySelf->matricola, mySelf->voto_AdE, mySelf->nof_invites, mySelf->max_reject, mySelf->nof_elems, getppid());
                WAITING_EVERYONE;
                /* getting a private copy of the list of all students */
                for(int student_number = 0; student_number < POP_SIZE; student_number++) {
                    student_list[student_number][0] = publicBoard->data[student_number][2];
                    student_list[student_number][1] = publicBoard->data[student_number][3];
                }
                /* bubble sort to sort the list by grade */
                int temp[2];
                for(int n1=0; n1<POP_SIZE; n1++){
                    for(int n2=0; n2<POP_SIZE-n1-1; n2++){
                        if(student_list[n2][1]>student_list[n2+1][1]) {  //Scambio valori
                            temp[0] = student_list[n2][0];
                            temp[1] = student_list[n2][1];
                            student_list[n2][0]=student_list[n2+1][0];
                            student_list[n2][1]=student_list[n2+1][1];
                            student_list[n2+1][0]=temp[0];
                            student_list[n2+1][1]=temp[1];
                        }
         		    }
                }
                /* looking up my self in the sorted student list */
                myPosition = findInMatrix(mySelf->matricola, student_list, POP_SIZE, 0);
                /* check if someone wrote me */
                my_msgid = msgget(mySelf->matricola, 0666 | IPC_CREAT);
                int test = 0;
                for(int j = myPosition + 1, invite = 0; invite < mySelf->nof_invites && j < POP_SIZE; j++) {
                    while((test = msgrcv(my_msgid, &message, sizeof(message), INVITE, IPC_NOWAIT)) != -1 && elem < mySelf->nof_elems) {
                        friend_id = msgget(message.whoAmI[0][MATRICOLA], 0666 | IPC_CREAT);
                        if(elem < mySelf->nof_elems) {
                            ADD_TO_GROUP(elem);
                            elem++;
                            message.type = ACCEPT;
                        fprintf(stderr,"accepting %d\n", message.whoAmI[0][MATRICOLA]);
                        }
                        else {
                            myGroup->closed = 1;
                            message.type = REFUSE;
                        fprintf(stderr,"refusing %d\n", message.whoAmI[0][MATRICOLA]);
                        }
                        SETUP_MESSAGE;
                        msgsnd(friend_id, &message, sizeof(message),IPC_NOWAIT);
                    }
                    if(student_list[j][MATRICOLA] % 2 == mySelf->matricola % 2) {
                        friend_id = msgget(student_list[j][MATRICOLA], 0666 | IPC_CREAT);
                        message.type = INVITE;
                        SETUP_MESSAGE;
                        msgsnd(friend_id, &message, sizeof(message), 0);
                        fprintf(stderr,"I'm %d inviting %d\n", mySelf->matricola, student_list[j][MATRICOLA]);
                        invite++;
                    }
                    fprintf(stderr,"%d %d %d %d I'm %d\n", test, elem, mySelf->nof_elems, errno, mySelf->matricola);
                }
                /* acceptiing all the invites for the remaining time ( minus 1 sec ) */
                while(msgrcv(my_msgid, &message, sizeof(message), ACCEPT, IPC_NOWAIT) != -1 && mySelf->nof_elems > elem) {
                    fprintf(stderr,"I'm %d accepting %d\n", mySelf->matricola, message.whoAmI[0][MATRICOLA]);
                    ADD_TO_GROUP(elem);
                    elem++;
                }
                while(msgrcv(my_msgid, &message, sizeof(message), INVITE, IPC_NOWAIT) != -1 && mySelf->nof_elems > elem) {
                    fprintf(stderr,"I'm %d accepting %d\n", mySelf->matricola, message.whoAmI[0][MATRICOLA]);
                    ADD_TO_GROUP(elem);
                    elem++;
                    friend_id = message.whoAmI[0][MATRICOLA];
                    message.type = ACCEPT;
                    SETUP_MESSAGE;
                    msgsnd(friend_id, &message, sizeof(message), 0);
                }
                while(msgrcv(my_msgid, &message, sizeof(message), ACCEPT, IPC_NOWAIT) != -1 && mySelf->nof_elems > elem) {
                    fprintf(stderr,"I'm %d accepting %d\n", mySelf->matricola, message.whoAmI[0][MATRICOLA]);
                    ADD_TO_GROUP(elem);
                    elem++;
                }
                /* reoving from the queue all the refuse */
                while(msgrcv(my_msgid, &message, sizeof(message), REFUSE, IPC_NOWAIT) != -1 && (errno != EAGAIN || errno != ENOMSG));
                myGroup->closed = 1;
                msgctl(my_msgid, IPC_RMID, NULL); 
                /* exiting with the grade of the group and whetere the group is closed or not */
                fprintf(stderr, "%d\n",max_grade(myGroup, mySelf->nof_elems));
                exit(EXIT_SUCCESS);
                break;
            default:
                /* teacher init*/
                all_student[i] = student_id;
                /* It will place all the students in the same process group (of the first student) */
                setpgid(all_student[i], all_student[0]); 
                break;
        }
    }
    READY_SET_GO;
    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0)
    printf("%d: child %d exited with status %d\n", (int)getpid(), corpse, status);
    /* closing and unlink the semaphores */
    sem_close(mutex);
    sem_unlink("/nof_elem");
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
int findInMatrix(int data, int** array, int length, int position) {
    for(int i = 0; i < length; i++)
        if (array[i][position] == data) return i;

    return -1;
}
int max_grade(group myGroup, int size) {
    int max = 0;
    for(int i = 0; i < size; i++)
        if(max < myGroup->array[i]->voto_AdE) max = myGroup->array[i]->voto_AdE;
    return max;
}