#include "headers.h"
#include"structs.h"
int getProcess()
{
    struct MessageBuffer message;
    int msgqKey;
    key_t key = ftok("keyFile", msgqKey);
    int msgq = msgget(key, IPC_CREAT | 0666);
        if (msgrcv(msgq, &message, sizeof(message.process), 1, IPC_NOWAIT) == -1)
        {
            perror("Error in receiving process from message queue");
        }
        printf("Scheduler received procces with pid %d from message queue at time %d",message.process.id,getClk());

}
int main(int argc, char *argv[])
{
    initClk();

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
