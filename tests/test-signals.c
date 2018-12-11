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

sig_atomic_t sigusr1_count = 0;
void handle(int signum);
struct c {
    int counter;
};

int main(int argc, char const *argv[])
{   
    struct c *cc;
    //pid_t pid = fork();
    sem_t *sem, *t;
    int m_id;
    m_id = shmget(IPC_PRIVATE, sizeof(*cc), 0600);
    cc = shmat(m_id, NULL, 0);
    cc->counter =0;
    sem = sem_open("/test", O_CREAT,  0644, 1);
    t = sem_open("/dio", O_CREAT,  0644, -1);
    signal(SIGUSR1, handle);

    pid_t all_child[5], pid;
    for(int i = 0 ; i < 5; i++)
        switch(pid = fork()) {
            case 0:
                sem_wait(sem);
                printf("ready %d son of %d\n", getpid(), getppid());
                cc->counter++;
                printf("i'm %d cc is %d", getpid(), cc->counter);
                sem_post(sem);
                sem_wait(t);
                printf("ok %d\n",getpid());
                break;
            default:
                while(cc->counter != 5) {sleep(1);printf("%d\n", cc->counter);}
                    if(cc->counter == 5)
                        for(int i = 0; i < 5; i++) sem_post(t);
                all_child[i] = pid;
                break;
        }
                
                    /* code */
    return 0;
}

void handle(int signum) {;}