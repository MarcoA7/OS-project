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

#define STATE 0

#ifndef POP_SIZE
#define POP_SIZE 4
#endif

#ifndef SIM_TIME
#define SIM_TIME 3
#endif
pid_t all_student[POP_SIZE]; /* array of all students */

struct msg_s { 
    long type; 
    int whoAmI[1][STUDENT_FIELDS]; 
} message;

int randomValue(int seed, int lower_bound, int upper_bound);
void sim_alarm_handler(int signum);
void reminder(int signum); /* friendly reminder that the time is running low */
void do_nothing(int signum);
int enough = 1; /* each student will have a timer set when it's time to speed up */

int main(int argc, char const *argv[])
{
    SET_UP_SYNC_MECH;
    POST_SENDING_MECH;
    int turn_counter = 0;
    sem_t *mutex;
    int status;
    int sum; //total of all students
    int elem = 0; //to identify how many students I have if my group
    int my_msgid, friend_id;
    pid_t student_id;
    FILE* config;
    struct student* mySelf;
    struct student* friend;
    group myGroup; //the group of the project
    int myPosition; //my position inside the list of all students
    int **student_list; //list of all student
    int got_invite; /* did I get any invite? */
    int im_free = 1; /* I'm in no group therefore I'm the leader of MY OWN group*/
    int can_decline; /* A variable which holds the REAL number of max_rejects */
    int score; /* group score */

    student_list = (int**)malloc(POP_SIZE * sizeof(int*));
    for(int i = 0; i < POP_SIZE; i++)
        student_list[i] = (int*) malloc(2 * sizeof(int));

    //allocating memory for the group;
    myGroup = malloc(sizeof(myGroup));
    
    //initialization of the semaphore
    mutex = sem_open("/accessing the board", O_CREAT, 0644, 1);
    //sem_unlink("accessing the board");

    // it has to become a shared variable
    MatrixF publicBoard;

    /* allocating memory for the list */
    student_list = (int**)malloc(POP_SIZE * sizeof(int*));
    for(int i = 0; i < POP_SIZE; i++)
        student_list[i] = (int*) malloc(2 * sizeof(int));

    /* changing the default
    * alarm signal handler */
    signal(SIGALRM, sim_alarm_handler);

    #if POSIX(STATE)
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

    /* Introduction */

    for(int i = 0; i < POP_SIZE; i++) {
        switch((student_id = fork())) {
            case 0:
                fprintf(stderr, "DEBUG %d\n", getpid());
                    /* student init */
                signal(SIGALRM, reminder);
                mySelf = malloc(sizeof(*mySelf));
                friend = malloc(sizeof(*friend));
                mySelf->matricola = randomValue(i, 11111, 900000);
                mySelf->voto_AdE = randomValue(i, 18, 30);
                mySelf->nof_invites = publicBoard->data[POS_NOF_INVITES][0];
                mySelf->max_reject = publicBoard->data[POS_MAX_REJECTS][0];
                sem_wait(mutex);
                //the semaphore turns red.
                /* critical section */
                fprintf(stderr, "DEBUG %d\n", getpid());
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
                    publicBoard->data[i][3] = mySelf->voto_AdE;
                //the semaphore turn green.
                sem_post(mutex);
                /* setting up an empty group */
                fprintf(stderr, "DEBUG %d\n", getpid());
                myGroup->array = malloc(mySelf->nof_elems * sizeof(*mySelf));
                for(int i = 0; i < mySelf->nof_elems-1; i++)
                    myGroup->array[i] = malloc(sizeof(struct student));
                myGroup->closed = 0; /* the group is open */

                fprintf(stderr, "DEBUG %d\n", getpid());
                fprintf(stderr, "I'm %d ready %d grades are %d max invites are: %d max rejects are: %d my team will have: %d people. My teacher is %d\n", getpid(),mySelf->matricola, mySelf->voto_AdE, mySelf->nof_invites, mySelf->max_reject, mySelf->nof_elems, getppid());
                WAITING_EVERYONE;
                alarm(SIM_TIME/2); /* Setting up a second alarm to be on time */
                /* getting a private copy of the list of all students */
                for(int student_number = 0; student_number < POP_SIZE; student_number++) {
                    if((int)(publicBoard->data)[student_number][2] % 2 == mySelf->matricola % 2) {
                        student_list[turn_counter][0] = publicBoard->data[student_number][2];
                        student_list[turn_counter][1] = publicBoard->data[student_number][3];
                        turn_counter++;
                    }
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
                myPosition = findInMatrix(mySelf->matricola, student_list, POP_SIZE, 0);
                /* check if someone wrote me */
                my_msgid = msgget(mySelf->matricola, 0666 | IPC_CREAT);
                can_decline = fmin(mySelf->max_reject, myPosition - 1); /* it is the mininum number between max_rejects and how many student are below me */
                sem_wait(mutex); /* One process at the time is able to send invite and texting back */
                for(int j = myPosition + 1, invite = 0; invite < mySelf->nof_invites && j < POP_SIZE; j++) {
                    while((got_invite = msgrcv(my_msgid, &message, sizeof(message), INVITE, IPC_NOWAIT)) != -1  && can_decline){
                        friend_id = msgget(message.whoAmI[0][MATRICOLA], 0666 | IPC_CREAT);
                        message.type = REFUSE;
                        SETUP_MESSAGE;
                        msgsnd(friend_id, &message, sizeof(message), IPC_NOWAIT);
                        can_decline--;
                    }
                    if(got_invite != -1 && !can_decline) {
                        friend_id = msgget(message.whoAmI[0][MATRICOLA], 0666 | IPC_CREAT);
                        message.type = ACCEPT;
                        SETUP_MESSAGE;
                        msgsnd(friend_id, &message, sizeof(message),IPC_NOWAIT);
                        im_free--;
                            fprintf(stderr, "im %d %d accepted his offer\n",mySelf->matricola, message.whoAmI[0][MATRICOLA]);
                        break;
                    }
                    friend_id = msgget(student_list[j][MATRICOLA], 0666 | IPC_CREAT);
                    message.type = INVITE;
                    SETUP_MESSAGE;
                    fprintf(stderr, "im %d inviting %d\n", mySelf->matricola, student_list[j][MATRICOLA]);
                    msgsnd(friend_id, &message, sizeof(message), 0);
                        //fprintf(stderr,"I'm %d at position %d inviting %d\n", mySelf->matricola, myPosition, student_list[j][MATRICOLA]);
                    invite++;                    
                }
                sem_post(mutex);
                EVERYONE_SENT_THEIR_MESSAGES; /* everyone holds their position */
                if(im_free) { 
                    /* If I'm the leader of my group I accept all the invitation I receivED
                    *  I will never get another invite
                    * */
                    while(msgrcv(my_msgid, &message, sizeof(message), INVITE, IPC_NOWAIT) != -1 && elem < mySelf->nof_elems - 1){
                        friend_id = msgget(message.whoAmI[0][MATRICOLA], 0666 | IPC_CREAT);
                        message.type = ACCEPT;
                        SETUP_MESSAGE;
                        msgsnd(friend_id, &message, sizeof(message), IPC_NOWAIT);
                    }
                    /* until there is space in my group and there is time add people who accepted */
                    while(enough && elem < mySelf->nof_elems - 1) {
                        if (msgrcv(my_msgid, &message, sizeof(message), ACCEPT, IPC_NOWAIT) != -1 && elem < mySelf->nof_elems - 1) {
                            ADD_TO_GROUP(elem);
                            elem++;
                        }
                    }    
                }
                /*this is a tecnical section
                * the student has no power here but the SIGINT
                * must be disabled to ensure everyone exit with their score
                * */
                signal(SIGINT, do_nothing); 
                /* need to disable the main alarm handler */
                myGroup->closed = 1;
                msgctl(my_msgid, IPC_RMID, NULL); 
                sem_wait(mutex); /* One process at the time */
                    if(elem > -1) {
                        fprintf(stderr, "I'm %d my grade is %d elem is: %d\n", mySelf->matricola, mySelf->voto_AdE, elem);
                    for(int i = 0; i < mySelf->nof_elems-1; i++)
                        fprintf(stderr, "mat %d grade %d\n",myGroup->array[i]->matricola, myGroup->array[i]->voto_AdE);
                sem_post(mutex);
                
                }
                /* exiting with the grade of the group */

                /* if I'm alone and I'm not alone in the turn 
                *   my score won't be use for the mean since 
                * I'm already in someone's else group */
                if(elem == 0 && turn_counter > 1) exit(-1);

                else if(elem > 0) {
                    /* if I reached my dream I won't get penalties */
                    score = max_grade(myGroup, mySelf->voto_AdE, mySelf->nof_elems-1) + (elem == mySelf->nof_elems - 1 ? 0 : -3);
                    fprintf(stderr, "I'm %d and my group scored %d\n",mySelf->matricola, score);
                    exit(score);
                }
                /* If I'm alone in the turn my group is composed just by myself */
                else if(turn_counter > 1) exit(mySelf->voto_AdE);
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

    MERGE;

    int corpse;
    int group_counter = 0;
    float mean = 0;
    while ((corpse = wait(&status)) > 0)
        if((score = WEXITSTATUS(status)) != 255 ){
            mean += score; 
            group_counter++;
        }
    mean /= group_counter;
    printf("There was %d group and the mean is: %.2f\n", group_counter, mean);
    //printf("%d: child %d exited with status %d\n", (int)getpid(), corpse, WEXITSTATUS(status));
    /* closing and unlink the semaphores */
    sem_close(mutex);
    sem_unlink("/nof_elem");
    #if !POSIX(STATE)
    shmctl(m_id, IPC_RMID, NULL);
    #endif
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
void reminder(int signum) {
    /* it will terminate all the process in the same group as the first student */
  enough = 0;
}
void do_nothing(int signum) {;}