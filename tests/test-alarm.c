#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
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

#define POP_SIZE 6

void handle(int signum);
pid_t child_pid[POP_SIZE], pid;
int j;
int main(int argc, char const *argv[])
{
            signal(SIGALRM, handle);
    for(j = 0; j < POP_SIZE; j++)
    switch( pid = fork()) {
        case 0:
            for(int i = 0; 1; i++) {
                fprintf(stderr, "%d\n", i);
            }
            exit(EXIT_SUCCESS);
            break;
        default:
            child_pid[j] = pid;
            setpgid(child_pid[j], child_pid[0]);
        break;
    }
    alarm(5);
    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0)
    printf("%d: child %d exited with status %d\n", (int)getpid(), corpse, status);
    printf("everyone died.\n");
    return 0;
}
void handle(int signum) {
    kill(-child_pid[0], SIGINT);
}