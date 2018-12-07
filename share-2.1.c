#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define SIZE 4096
#define RIGHE 30
#define COLONNE 30

typedef struct mat {
	int data[30][30];
} mat;

typedef mat* Matrix;

int main(int argc, char const *argv[])
{
    int shm_fd;
	void *ptr;
    int shared_seg_size = (sizeof(Matrix));
	Matrix myMatrix;
	
	
	#if 0
	/* create the shared memory segment */
	shm_fd = shm_open("shared", O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory segment */
	ftruncate(shm_fd,SIZE);

	/* now map the shared memory segment myMatrix the address space of the process */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}
	#elif 0
	printf("TEST\n");
	int shfd = shm_open("project", O_RDWR|O_CREAT, 0750);
	ftruncate(shfd, SIZE);
	if (shfd<0) { perror("shm_open"); exit(EXIT_FAILURE); }
	printf("TEST\n");
	ptr = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shfd, 0);
	printf("TEST\n");
	if (ptr == MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); };
	sleep(9);

	#elif 1
	/* creating the shared memory object    --  shm_open()  */
    shm_fd = shm_open("project", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if (shm_fd < 0)
    {
        perror("In shm_open()");
        exit(1);
    }

    fprintf(stderr, "Created shared memory object %s\n", "project");

    /* adjusting mapped file size (make room for the whole segment to map)      --  ftruncate() */
    ftruncate(shm_fd, shared_seg_size);

    /* requesting the shared segment    --  mmap() */
    myMatrix = (Matrix)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (myMatrix == NULL)
    {
        perror("In mmap()");
        exit(1);
    }
	
	myMatrix->data[0][0] = 9;
	int i = 0;
	int j = 0;
	printf("%d\n", myMatrix->data[i][j]);
	#endif

	#if 1
    switch(fork()) {
        case 0:
            /* child stuff */
            printf("before modifing child: %d\n", myMatrix->data[0][0]);
			myMatrix->data[0][0] = 7;
            printf("child: %d\n", myMatrix->data[0][0]);
			exit(0);
            break;
        default:
            /* parent stuff */
			wait(NULL);
            printf("father: %d\n", myMatrix->data[0][0]);
	        //ptr += strlen(string);
            break;
    }

    
	#endif
	if (shm_unlink("project") != 0) {
        perror("In shm_unlink()");
        exit(1);
    }
    return 0;
}
