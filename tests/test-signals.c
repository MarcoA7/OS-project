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

#define GETPID printf("%d\n", getpid())
#define GETPPID printf("%d\n", getppid())

sig_atomic_t sigusr1_count = 0;
void handle(int signum);

int main(int argc, char const *argv[])
{   
    int c;
    //pid_t pid = fork();
    sem_t *sem, *t;
    sem = sem_open("/test", O_CREAT,  0644, 1);
    t = sem_open("/dio", O_CREAT,  0644, 2);
    signal(SIGUSR1, handle);
    int fd[2];
    pipe(fd);
    pid_t all_child[5], pid;
    for(int i = 0 ; i < 5; i++)
        switch(all_child[i] = fork()) {case 0: {
                if(i!=0) {
                    close(fd[0]); // close unused READ end
                    printf("OK\n");       // do some work                       
                    close(fd[1]); // close WRITE end, the last child
                                  // to close will cause the read()
                                  // of first child to unblock
                }
                if(i==0) { // first child    
                    close(fd[1]); // close unused WRITE end
                    // do some work                       
                    char x = 0;
                    fprintf(stderr, "1st Child's wait started %d\n",
                        getpid());
                    read(fd[0], &x, 1); // blocking call, until all
                                        // siblings close the WRITE
                                        // end
                    fprintf(stderr, "1st Child's wait over %d\n",
                        getpid());
                    close(fd[0]); // close READ  end
                }       
                fprintf(stderr, "Child %d ready\n", getpid());            
                exit(0);
                break;
            }
        }
    close(fd[1]);      
                    /* code */
    return 0;
}

void handle(int signum) {;}