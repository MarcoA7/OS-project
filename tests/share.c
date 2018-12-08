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

struct mat {
	int rows;
	int cols;
	int **data;
};
int main(int argc, char const *argv[])
{
    struct mat * myMat;
    TEST(__LINE__);
    
    myMat = malloc(sizeof(struct mat *));
    myMat->data = (int**)malloc(sizeof(int*) * 30);
    TEST(__LINE__);

    for(int i = 0; i < 30; i++) myMat->data[i] = (int *)malloc(sizeof(int) * 30);
    TEST(__LINE__);

    myMat->data[0][0] = -1;
    TEST(__LINE__);

    int m_id = shmget(IPC_PRIVATE, sizeof(*myMat), 0600);
    TEST(__LINE__);

    myMat = shmat(m_id, NULL, 0);
    TEST(__LINE__);

    myMat->data[0][0] = 9;
    TEST(__LINE__);

    switch(fork()) {
        case 0:
            myMat->data[0][0] = 4;
            printf("child: %d\n", myMat->data[0][0]);
            exit(0);
            break;
        default:
            wait(NULL);
            printf("father: %d\n", myMat->data[0][0]);
            break;

    }
    /* la zona è marcata per deallocazione
     * appena non c'è più nessun processo attaccato verrà de allocata
    */
    while(shmctl(m_id, IPC_RMID, NULL));
    return 0;
}
