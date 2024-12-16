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
int processcount = 0;
Queue *ProcessQueue;
pid_t clk_pid, scheduler_pid;

void readfile() {
    FILE *file = fopen("processes.txt", "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    ProcessQueue = create_queue();
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#') continue; // Skip comments

        // Allocate memory for a new process
        Process *p = (Process *)malloc(sizeof(Process));
        if (!p) {
            perror("Error allocating memory for process");
            exit(1);
        }

        sscanf(line, "%d\t%d\t%d\t%d", &p->id, &p->arrival_time, &p->runtime, &p->priority);
        processcount++;
        printf("\n%d\t%d\t%d\t%d\n",  p->id, p->arrival_time, p->runtime, p->priority);
        enqueue(ProcessQueue, p); // Pass the pointer to the queue
    }
    fclose(file);
}


int selectSchedulingAlgo(int scheduling_type, int quantum) {
    switch (scheduling_type) {
    case 1:
        printf("SJF Selected\n");
        break;
    case 2:
        printf("HPF Selected\n");
        break;
    case 3:
        printf("RR Selected with quantum %d\n", quantum);
        break;
    case 4:
        printf("Multiple level Feedback Loop Selected\n");
        break;
    }
    return 0;
}

int forkClkScheduler(int ProcessCount, int scheduling_type, int quantum) {
    // Fork clock process
    clk_pid = fork();
    if (clk_pid == -1) {
        perror("Error in fork (clock)");
        exit(-1);
    } else if (clk_pid == 0) {
        execl("./clk.out", "clk.out", NULL);
        perror("Error executing clock process");
        exit(-1);
    }

    // Fork scheduler process
    scheduler_pid = fork();
    if (scheduler_pid == -1) {
        perror("Error in fork (scheduler)");
        exit(-1);
    } else if (scheduler_pid == 0) {
        char buf1[10], buf2[10], buf3[10];
        sprintf(buf1, "%d", scheduling_type);
        sprintf(buf2, "%d", quantum);
        sprintf(buf3, "%d", ProcessCount);
        execl("./scheduler.out", "scheduler.out", buf1, buf2, buf3, NULL);
        perror("Error executing scheduler process");
        exit(-1);
    }
    return 0; // Parent process continues
}

int main(int argc, char *argv[]) {
    signal(SIGINT, clearResources);
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <scheduling_algorithm> [quantum]\n", argv[0]);
        exit(1);
    }

    readfile();

    int scheduling_type = atoi(argv[2]);
    int quantum = 0;
    if (scheduling_type == 3) {
        quantum = atoi(argv[3]);
    }
    selectSchedulingAlgo(scheduling_type, quantum);

    forkClkScheduler(processcount, scheduling_type, quantum);

    initClk();
    printf("Clock and Scheduler initialized.\n");
    int clk = getClk();
    printf("Current time from clock: %d\n", clk);
    //sleep(5);

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

    while (!isEmptyQueue(ProcessQueue)) {
        printf("Clk in process generator = %d", getClk());
        Process *process = dequeue(ProcessQueue); // Get pointer to process
        printf("\nProcess arrived at %d", process->arrival_time);

        while (process->arrival_time > getClk()) {}

        printf("\nCurrent CLK %d\n", getClk());

        struct MessageBuffer message;
        message.process = *process; // Copy process data
        message.mtype = 1;
        int sent = msgsnd(msgqid, &message, sizeof(message.process), 1);
        if (sent == -1) {
            perror("Error sending process to message queue");
        } else {
            printf("Process with ID %d sent at time %d\n", message.process.id, getClk());
        }

        free(process); // Free memory after sending
    }

    while(true) {} //busy wait

    destroyClk(true);
    
    return 0;
}

void clearResources(int signum) {
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("Error removing message queue");
    }
    if (clk_pid > 0) kill(clk_pid, SIGINT);
    if (scheduler_pid > 0) kill(scheduler_pid, SIGINT);
    destroyClk(true);
    exit(0);
}
