#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define TEST(x) printf("TEST %d\n", x);


int main(int argc, char const *argv[])
{
    int **myMat;
    myMat = (int**)malloc(sizeof(int*) * 30);
    for(int i = 0; i < 30; i++) myMat[i] = (int*)malloc(sizeof(int) * 30);
    myMat[0][0] = -1;
    printf("%d\n", myMat[0][0]);
    TEST(__LINE__);

    int m_id = shmget(IPC_PRIVATE, sizeof(**myMat), 0600);
    TEST(__LINE__);

    myMat = shmat(m_id, NULL, 0);
    TEST(__LINE__);
    
    myMat[0][0] = 9;
    printf("number: %d\n", myMat[0][0]);
    TEST(__LINE__);
    switch(fork()) {
        case 0:
            myMat[0][0] = 4;
            printf("child: %d\n", myMat[0][0]);
            exit(0);
            break;
        default:
            wait(NULL);
            printf("father: %d\n", myMat[0][0]);
            break;

    }
    /* la zona è marcata per deallocazione
     * appena non c'è più nessun processo attaccato verrà de allocata
    */
    while(shmctl(m_id, IPC_RMID, NULL));
    return 0;
}
