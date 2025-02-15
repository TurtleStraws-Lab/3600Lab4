//Moises Gonzalez
//2/13/2025
//3600
//
//
//Lab 4

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define BUFSIZE 200

int status;
char buf[BUFSIZE];

int main() {
    key_t ipckey;
    int mqid, myNum;
    int logfd;
    FILE *myinput;
    int received;
    struct {
        long type;
        char text[128];
    } mymsg;

    char pathname[128];
    logfd = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    myinput = fopen("myinput", "r");
    myNum = 18;
    getcwd(pathname, 128);
    strcat(pathname, "/foo");
    printf("%s \n", pathname); 
    ipckey = ftok(pathname, myNum);
    int shmid;
    if (ipckey == -1) {
        perror("ipckey error: ");
        exit(EXIT_FAILURE);
    }
    mqid = msgget(ipckey, IPC_CREAT | 0666);
    if (mqid < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }    
    shmid = shmget(ipckey, sizeof(int), IPC_CREAT | 0666);
    pid_t child = fork();
    if (child == 0) {
        int *shared = shmat(shmid, (void *)0, 0);
        *shared = 0;
        while(*shared == 0){
        usleep(1000);
        }
            received = msgrcv(mqid, &mymsg, sizeof(mymsg), 0, 0);
            sprintf(buf, "%i \n", *shared);
            if (received > 0) {
                write(logfd, buf, strlen(buf));
                memset(buf, 0, 100);
                sprintf(buf, "%s \n", mymsg.text);
                write(logfd, buf, strlen(buf));

            } 

        
        close(logfd);
        shmdt(shared);
        exit(0);
    } else {
        int *shared = shmat(shmid, (void *)0, 0);
        char prompt[] = "Enter a two-digit number: ";
        write(1, prompt, strlen(prompt));
        read(0,buf, 3);
        memset(mymsg.text, 0, 100);

        int number = atoi(buf);
        *shared = number;
        char prompt2[] = "Enter a word: ";
        write(1, prompt2 ,strlen(prompt2)); 
        read(0, mymsg.text, 20);
        mymsg.type = 1;
        msgsnd(mqid, &mymsg, sizeof(mymsg), 0);
        fscanf(myinput, "%2d", &number);
        fscanf(myinput, "%s", mymsg.text);
        fclose(myinput);
        wait(&status);
        printf("my child exited with exit code: %i \n", WEXITSTATUS(status));

        shmdt(shared);
        msgctl(mqid, IPC_RMID, NULL);
        shmctl(shmid, IPC_RMID, NULL);
    }


    return 0;
}
