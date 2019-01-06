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

#define SIM_TIME 4

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

void reminder(int signum);
void handler(int signum);
int test = 1;
pid_t child;
int main(int argc, char const *argv[])
{
    SET_UP_SYNC_MECH;
    signal(SIGALRM, handler);
    switch(child = fork()) {
        case 0:
        signal(SIGALRM, reminder);
        WAITING_EVERYONE;
        alarm(SIM_TIME -1);
        while(test) fprintf(stderr, "ok\n");
        fprintf(stderr, "im leaving\n");
        exit(EXIT_SUCCESS);
        break;
        default:
        break;
    }
    READY_SET_GO;
    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0)
    printf("%d: child %d exited with status %d\n", (int)getpid(), corpse, status);
    return 0;
    
}
void reminder(int signum) {
    test--;
}
void handler(int signum) {
    kill(child, SIGINT);
}