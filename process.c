#include "headers.h"

#define PROCESS_KEY 12
#define PROCESS_LOCK 6   //Type of processes received 
#define ERROR -1

/* Modify this file as needed*/

//process code; runned when process is in running state after the schedular forks the child process
int main()
{
    initClk();

    //creating a new message queue with its unique identifier
    key_t key_pid = ftok("keyfile", PROCESS_KEY);
    int msgq_id = msgget(key_pid, 0666 | IPC_CREAT);  //IPC_CREAT --> create a message queue if it does not exit, else return its ID

    //error check
    if(msgq_id == ERROR){
        perror("Cannot create message queue!\n");
        exit(Error); //exiting from process with error code -1
    }

    //TODO The process needs to get the remaining time from somewhere 
    MessageBuffer msgProcess;

    if(msgrcv(msgq_id, &msgProcess, sizeof(msgProcess) - sizeof(long), PROCESS_LOCK, !IPC_NOWAIT) == ERROR){
        perror("Error receiving message!\n");
        exit(Error);
    }

    while(msgProcess.process.pcb.remainingTime > 0){
        int dummy = msgrcv(msgq_id, &msgProcess, sizeof(msgProcess) - sizeof(long), PROCESS_LOCK, !IPC_NOWAIT);
    }

    exit(1);

    destroyClk(false);

    return 0;
}
