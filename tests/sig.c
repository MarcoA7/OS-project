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
    sem = sem_open("/ok", O_CREAT, 0);
    signal(SIGUSR1, handle);

    for(int i = 0; i < NUM_KIDS; i++) {
        switch(fork()) {
            case 0:
                fprintf(stderr, "ready %d from %d\n", getpid(), getppid());
                /* i would like that each child stop here untill everyone is ready */
                for(int j = 0; j < 10; j++) 
                fprintf(stderr, "lot of stuff\n");
                exit(0);
            break;
            default:
                /* unleashing the kids when everyone is ready */
                wait(NULL);
                fprintf(stderr, "OK\n");
                
            break;
        }
    }
    return 0;
}

void handle(int signum) {;}