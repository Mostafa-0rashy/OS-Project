#include "headers.h"
#include "structs.h"
#include <errno.h>

int getProcess(int processcount) {
    struct MessageBuffer message;
    key_t key = ftok("keyFile", 65);
    int msgqid = msgget(key, IPC_CREAT | 0666);

    if (msgqid == -1) {
        perror("Error creating message queue");
        exit(-1);
    }
    int processin=0;
    while(processin<=processcount)
    {
    printf("Attempting to receive message...\n");
    int r = msgrcv(msgqid, &message, sizeof(message.process), 1, !IPC_NOWAIT);
    printf("Scheduler received process with pid %d from message queue at time %d\n", message.process.id, getClk());
    processin++;

    }
}
int main(int argc, char *argv[]) {
    initClk();
    int processcount=atoi(argv[3]);
    printf("process count is %d",processcount);
    getProcess(processcount);
    return 0;
}
