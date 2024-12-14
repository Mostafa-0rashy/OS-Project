#include "headers.h"

#define PROCESS_GEN_SCHEDULER 6
#define SCHEDULER_Q_KEY 65
#define SCHEDULER_LOCK 8
#define Error -1

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
        Process *p = (Process *)malloc(sizeof(Process)); // Allocate memory for each Process
        if (!p) {
            perror("Memory allocation failed for process");
            exit(1);
        }
        sscanf(line, "%d\t%d\t%d\t%d", &p->id, &p->arrival_time, &p->runtime, &p->priority);
        processcount++;
        enqueue(ProcessQueue, p);  // Pass the pointer to the queue
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
}

int forkClkScheduler(int ProcessCount, int scheduling_type, int quantum) {
    // Fork clock process
    clk_pid = fork();
    if (clk_pid == Error) {
        perror("Error in fork (clock)");
        exit(Error);
    } else if (clk_pid == 0) {
        execl("./clk.out", "clk.out", NULL);
        perror("Error executing clock process");
        exit(Error);
    }

    // Fork scheduler process
    scheduler_pid = fork();
    if (scheduler_pid == Error) {
        perror("Error in fork (scheduler)");
        exit(Error);
    } else if (scheduler_pid == 0) {
        char buf1[10], buf2[10], buf3[10];
        sprintf(buf1, "%d", scheduling_type);
        sprintf(buf2, "%d", quantum);
        sprintf(buf3, "%d", ProcessCount);
        execl("./scheduler.out", "scheduler.out", buf1, buf2, buf3, NULL);
        perror("Error executing scheduler process");
        exit(Error);
    }
    return 0; // Parent process continues
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
    if (scheduling_type == 3) {
        quantum = atoi(argv[3]);
    }

    selectSchedulingAlgo(scheduling_type, quantum);

    // 4. Fork scheduler process
    forkClkScheduler(processcount, scheduling_type, quantum);

    // Initialize clock for current process
    initClk();
    printf("Clock and Scheduler initialized.\n");
    int clk = getClk();
    printf("Current time from clock: %d\n", clk);
    sleep(5);

    // 5. Initialize message queue
    key_t key = ftok("keyFile", SCHEDULER_Q_KEY);
    if (key == Error) {
        perror("Error creating key");
        exit(Error);
    }

    // Message queue creation
    msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid == Error) {
        perror("Error creating message queue");
        exit(Error);
    }

    // 6. Send processes to scheduler at the appropriate time
    while (!isEmptyQueue(ProcessQueue)) {
        Process *process = dequeue(ProcessQueue);  // Get the pointer to the process
        printf("\nprocess arrived at %d", process->arrival_time);
        while (process->arrival_time > getClk()) {
        }
        printf("\nCurrent CLK %d\n", getClk());

        struct MessageBuffer message;
        message.process = *process;  // Dereference the pointer to copy the process data
        message.mtype = PROCESS_GEN_SCHEDULER;  // message type: 6 --> PROCESSGEN_SCHEDULER
        int sent = msgsnd(msgqid, &message, sizeof(message.process), IPC_NOWAIT);

        if (sent == Error) {
            perror("Error sending process to message queue");
        } else {
            printf("Process with ID %d sent at time %d\n", message.process.id, getClk());
        }
    }

    // Cleanup and exit
    destroyClk(true);  // Restart clock
    return 0;
}

void clearResources(int signum) {
    // Cleanup message queue
    if (msgctl(msgqid, IPC_RMID, NULL) == Error) {
        perror("Error removing message queue");
    }
    // Terminate clock and scheduler processes
    if (clk_pid > 0) kill(clk_pid, SIGINT);
    if (scheduler_pid > 0) kill(scheduler_pid, SIGINT);
    destroyClk(true);
    exit(0);
}
