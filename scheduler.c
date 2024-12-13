#include "headers.h"
#include "structs.h"
#include <errno.h>

int getProcess() {
    struct MessageBuffer message;
    key_t key = ftok("keyFile", 65);
    int msgqid = msgget(key, IPC_CREAT | 0666);

    if (msgqid == -1) {
        perror("Error creating message queue");
        exit(-1);
    }

    printf("Attempting to receive message...\n");

    int r = msgrcv(msgqid, &message, sizeof(message.process), 1, IPC_NOWAIT);
    printf(" r is %d\n", r);
    fflush(stdout);

    // Retry if no message was found
    while (r == -1 && errno == ENOMSG) {
        printf("No messages yet, retrying...\n");
        sleep(1);  // Wait for a second before retrying
        r = msgrcv(msgqid, &message, sizeof(message.process), 1, IPC_NOWAIT);
    }

    printf("Scheduler received process with pid %d from message queue at time %d\n", message.process.id, getClk());
}

int main(int argc, char *argv[]) {
    getProcess();
    return 0;
}
