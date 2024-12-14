#include "headers.h"
#include "structs.h"
#include <errno.h>

struct Process getProcess(int processcount) {
    struct MessageBuffer message;
    key_t key = ftok("keyFile", 65);
    int msgqid = msgget(key, IPC_CREAT | 0666);

    if (msgqid == -1) {
        perror("Error creating message queue");
        exit(-1);
    }
    
    printf("Attempting to receive message...\n");
    int r = msgrcv(msgqid, &message, sizeof(message.process), 1, !IPC_NOWAIT);
    printf("Scheduler received process with pid %d from message queue at time %d\n", message.process.id, getClk());

    return message.process;
    
}
int main(int argc, char *argv[]) {
    initClk();
    int AlgoType=atoi(argv[1]);
    int quantum=atoi(argv[2]);
    int processcount=atoi(argv[3]);
    int processin=0;
    printf("process count is %d",processcount);
    while (processin<=processcount)
    {
       Process ProcessArrived= getProcess(processcount);
        processin++;
        switch (AlgoType)
        {
        case 1:
                SJF(ProcessArrived);
                break;
            case 2:
                HPF(ProcessArrived);
                break;
            case 3:
                RR(ProcessArrived,quantum);
                break;
            case 4:
                Multilevel(ProcessArrived);

            default:
                printf("invalid algorithm");
                break;
    }
    }
    return 0;
}
