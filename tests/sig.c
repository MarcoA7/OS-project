#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>

#define NUM_KIDS 4

void handle(int signum);

int main(int argc, char const *argv[])
{
    sem_t* sem;
    int p_pipe[2], c_pipe[2];
    char c= 0;
    sem = sem_open("/ok", O_CREAT, 0);
    signal(SIGUSR1, handle);
    pipe(p_pipe);
    pipe(c_pipe);
    for(int i = 0; i < NUM_KIDS; i++) {
        switch(fork()) {
            case 0:
                fprintf(stderr, "ready %d from %d\n", getpid(), getppid());
                /* i would like that each child stop here untill everyone is ready */
                close(p_pipe[0]);
                close(p_pipe[1]);
                close(c_pipe[1]);
                read(c_pipe[0], &c, 1);
                close(c_pipe[0]);
                for(int j = 0; j < 10; j++) 
                fprintf(stderr, "lot of stuff\n");
                exit(0);
            break;
            default:
            break;
        }
    }
    close(p_pipe[1]);
    read(p_pipe[0], &c, 1);
    printf("%d: %d children started\n", (int)getpid(), NUM_KIDS);
    close(c_pipe[0]);
    close(c_pipe[1]);
    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0)
    printf("%d: child %d exited with status 0x%.4X\n", (int)getpid(), corpse, status);
    return 0;
}

void handle(int signum) {;}