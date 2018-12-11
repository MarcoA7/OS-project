/* POSIX */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/shm.h>
#include<sys/wait.h> 

int main(int argc, char const *argv[])
{
    sem_t *sem2; // Second semaphore

    // create, initialize semaphores
    
    sem2 = sem_open("/semaphore2", O_CREAT,  0644, 1);

    for(int i = 0; i < 3; i++) {
        if(fork()==0) {
            /***/
                sem_wait(sem2);
                printf("child %d of %d %d\n",getpid(), getppid(),  i);
                sem_post(sem2);
                /***/
        }
        else printf("father %d\n", getpid());
    }
    //sem_destroy(&sem);
    /* code */
    return 0;
}
