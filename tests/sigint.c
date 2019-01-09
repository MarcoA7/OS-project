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

void handler(int signum);

pid_t child;
int main(int argc, char const *argv[])
{
    signal(SIGALRM, handler);

    switch(child = fork()) {
        case 0:
            while(1);
            exit(EXIT_SUCCESS);
            break;
    }
    alarm(6);
    int status;
    int corpse;
    int score;
    while ((corpse = wait(&status)) > 0)
        if((score = WEXITSTATUS(status)) != 255 ){
            printf("%d\n",status);
        }
    return 0;
} 
void handler(int signum) {
    kill(child, SIGINT);
}