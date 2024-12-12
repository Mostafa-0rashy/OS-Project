#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"
#include "structs.h"
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

void clearResources(int);
int msgqid;
int processcount=0;
Queue *ProcessQueue;
pid_t clk_pid, scheduler_pid;
void readfile()
{
FILE *file = fopen("processes.txt", "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    ProcessQueue = create_queue();
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#') continue; // Skip comments
        Process p;
        sscanf(line, "%d\t%d\t%d\t%d", &p.id, &p.arrival_time, &p.runtime, &p.priority);
        processcount++;
        enqueue(ProcessQueue, p);
    }
    fclose(file);
}

int selectSchedulingAlgo(int scheduling_type,int quantum)
{
    switch (scheduling_type)
    {
    case 1:
    printf("SJF Selected");
    break;
    case 2:
    printf("HPF Selected");
    break;
    case 3:
        printf("RR Selected with quantum %d",quantum);
        break;
    case 4:
        printf("Multiple level Feedback Loop Selected");
        break;
    }

}



int forkClkScheduler(int ProcessCount,int scheduling_type,int quantum)
{
    clk_pid = fork();
    if (clk_pid == -1) {
        perror("Error in fork (clock)");
        exit(-1);
    } else if (clk_pid == 0) {
        execl("./clk.out", "clk.out", NULL);
        sleep(1);
        printf("in clk child");
        perror("Error executing clock process");
        exit(-1);
    }
    else
    {
        scheduler_pid = fork();
    if (scheduler_pid == -1) {
        perror("Error in fork (scheduler)");
        exit(-1);
    } else if (scheduler_pid == 0) {
        char buf1[10], buf2[10], buf3[10];
        sprintf(buf1, "%d", scheduling_type);
        sprintf(buf2, "%d", quantum);
        sprintf(buf3, "%d", processcount); // Send process count
        execl("./scheduler.out", "scheduler.out", buf1, buf2, buf3, NULL);
        perror("Error executing scheduler process");
        exit(-1);
    }
    }
}
int SendMessage(int msgid, struct Process process) {
    struct MessageBuffer message;
    message.process = process;
    return msgsnd(msgid, &message, sizeof(message.process), !IPC_NOWAIT);
}







int main(int argc, char *argv[]) {
    signal(SIGINT, clearResources);
    // Ensure argument validity
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <scheduling_algorithm> [quantum]\n", argv[0]);
        exit(1);
    }

    // 1. Read input file
    readfile();

    // 2. Parse scheduling algorithm and parameters
    int scheduling_type = atoi(argv[2]);
    int quantum = 0;
    if(scheduling_type==3)
    {
        quantum=atoi(argv[3]);
    }
    selectSchedulingAlgo(scheduling_type,quantum);

    // 4. Fork scheduler process
    initClk();
   forkClkScheduler(processcount,scheduling_type,quantum);
    // Initialize clock for current process
    printf("Clock and Scheduler initialized.\n");
    int clk = getClk();
    printf("Current time from clock: %d\n", clk);


    // 5. Initialize message queue
    key_t key = ftok("keyFile", 65);
    if (key == -1) {
        perror("Error creating key");
        exit(-1);
    }
    msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("Error creating message queue");
        exit(-1);
    }

    // 6. Send processes to scheduler at the appropriate time
    printf("Current time from clock: %d\n", clk);
    while (!isEmptyQueue(ProcessQueue)) {
        Process process = dequeue(ProcessQueue);
        printf(" process: %d",process.arrival_time);
        while (process.arrival_time > getClk()) {
            sleep(1);
            printf("\nIn while loop %d\n", clk);
        }
        if (SendMessage(msgqid, process) == -1) {
            perror("Error sending process to scheduler");
            exit(-1);
        }
        printf("Process ID %d sent at time %d\n", process.id, getClk());
    }

    // Cleanup and exit
    return 0;
}

void clearResources(int signum) {
    // Cleanup message queue
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("Error removing message queue");
    }
    // Terminate clock and scheduler processes
    if (clk_pid > 0) kill(clk_pid, SIGINT);
    if (scheduler_pid > 0) kill(scheduler_pid, SIGINT);
    destroyClk(true);
    exit(0);
}
