#include "student.h"
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TEST(x) printf("riga %d\n",x);
#define RIGA __LINE__
#if 0
struct msg { 
    long type; 
    student whoAmI;
} message;
#elif 1
struct student {
    int matricola;
    int voto_AdE;
};
struct msg {
    long type;
    struct student whoAmI;
} message;
#else
struct mesg_buffer { 
    long mesg_type; 
    char text[100]; 
} message;

#endif
int main(int argc, char const *argv[])
{
    int msgid;
    int result;
    pid_t childId;
    key_t key;
    //message.whoAmI = malloc(sizeof(*message.whoAmI));
    #if 1
    switch(childId = fork()) {
        case 0:
            key= getpid();
            msgid = msgget(key, 0666 | IPC_CREAT);
            message.whoAmI.matricola = 777;
            message.whoAmI.voto_AdE = 19;
            message.type = 1;
            msgsnd(msgid, &message, sizeof(message), 0);
            printf("child is sending %d and %d\n", message.whoAmI.matricola, message.whoAmI.voto_AdE);
            exit(0);
        default:
            wait(NULL);
            key = childId;

            msgid = msgget(key, 0666 | IPC_CREAT);
            msgrcv(msgid, &message, sizeof(message), 1, 0);
            printf("Matricola is : %d and grade is %d \n",  message.whoAmI.matricola, message.whoAmI.voto_AdE);
            msgctl(msgid, IPC_RMID, NULL); 
            break;

    }

    #elif 0
    message.whoAmI = malloc(sizeof(student));
    message.whoAmI.matricola = 100;
    printf("%d", message.whoAmI.matricola);
    #else
    switch(childId = fork()) {
        case 0:
            msgid = msgget(getpid(), 0666 | IPC_CREAT);
            message.mesg_type = 1; 
            gets(message.text);
            msgsnd(msgid, &message, sizeof(message), 0);
            exit(0);
            break;
        default:
            wait(NULL);
            msgid = msgget(childId, 0666 | IPC_CREAT);
            // msgrcv to receive message 
            msgrcv(msgid, &message, sizeof(message), 1, 0); 
        
            // display the message 
            printf("Data Received is : %s \n", message.text); 
            break;    
    
    }
    
    #endif
    return 0;
}
