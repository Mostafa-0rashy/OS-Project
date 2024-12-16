#include "headers.h"

#define PROCESS_GEN_SCHEDULER 1
#define SCHEDULER_Q_KEY 65
#define ERROR -1
#define EMPTY_MESSAGE_QUEUE -1
#define EMPTY_READY_Q 0
#define TRUE_CONDITION 1
#define PROCESS_KEY 12
#define PROCESS_LOCK 6
#define NEW_PROCESS -1
#define STARTED_STATE 0
#define RESUMED_STATE 1
#define STOPPED_STATE 2
#define FINISHED_STATE 3
#define CHILD_PROCESS 0
#define sleep_seconds 200000


//global variables
Process* runningProcess = NULL;  //points to current running process
MessageBuffer message; //hold the message received from the message queue of the schedular
int msgq_id; //global variable of message queue (running queue of the scheduling algorithm)
int key_id;
int quantum;
int Terminated_Processes = 0; //counts the number of terminated processes
int ProcessCount; //Stores the total number of processes to be scheduled




//helper function to recieve the process from the message queue
void receiveProcesses(Queue* ready_queue) {
    MessageBuffer RR_msg;
    int msgR;

    key_t key = ftok("keyFile", SCHEDULER_Q_KEY);
    int msgqid = msgget(key, IPC_CREAT | 0666);

    // Continue receiving messages as long as there are processes in the queue
    while (1) {
        msgR = msgrcv(msgqid, &message, sizeof(message.process), 1, IPC_NOWAIT);
        
        if (msgR == -1) {
            // Check if the message queue is empty
            if (errno == ENOMSG) {
                //printf("\nMsg Q is empty\n");
                return;  // Break the loop if the queue is empty
            } else {
                perror("Error receiving message");
                return;  // Exit the loop if there is another error
            }
        }

        // Create a new process from the received message
        Process* new_process = Create_Process(message.process.id, message.process.arrival_time, message.process.runtime, message.process.priority);
        //printf("Received process id %d at time %d\n", new_process->id, getClk());

        // Add the process to the ready queue
        enqueue(ready_queue, new_process);
        //printf("Queue size: %d\n", sizeQueue(ready_queue));
    }
}









//helper function to print the process current state
const char* GetStateName(int state) {
    switch (state) {
        case NEW_PROCESS:
            return "NEW";
        case STARTED_STATE:
            return "STARTED";
        case RESUMED_STATE:
            return "RESUMED";
        case STOPPED_STATE:
            return "STOPPED";
        case FINISHED_STATE:
            return "FINISHED";
        default:
            return "UNKNOWN";
    }
}




void PrintProcessState(Process* process) {
    printf("Process ID: %d\n", process->id);
    printf("State: %s\n", GetStateName(process->pcb.state)); // Helper function to get state name
    printf("Arrival Time: %d\n", process->arrival_time);
    printf("Start Time: %d\n", process->startTime);
    printf("Finish Time: %d\n", process->FinishTime);
    printf("Remaining Time: %d\n", process->pcb.remainingTime);
    printf("Turnaround Time: %d\n", process->pcb.TurnaroundTime);
    printf("Weighted Turnaround Time: %.2f\n", process->pcb.WeightedTurnaroundTime);
    printf("Waiting Time: %d\n", process->pcb.waitingTime);
    printf("----------------------------------\n");
}


void WriteProcessStateToFile(Process* process) {
    FILE* file = fopen("scheduler_out.txt", "a"); // Open the file in append mode
    if (file == NULL) {
        perror("Failed to open scheduler_out.txt");
        return;
    }

    fprintf(file, "Process ID: %d\n", process->id);
    fprintf(file, "State: %s\n", GetStateName(process->pcb.state)); // Helper function to get state name
    fprintf(file, "Arrival Time: %d\n", process->arrival_time);
    fprintf(file, "Start Time: %d\n", process->startTime);
    fprintf(file, "Finish Time: %d\n", process->FinishTime);
    fprintf(file, "Remaining Time: %d\n", process->pcb.remainingTime);
    fprintf(file, "Turnaround Time: %d\n", process->pcb.TurnaroundTime);
    fprintf(file, "Weighted Turnaround Time: %.2f\n", process->pcb.WeightedTurnaroundTime);
    fprintf(file, "Waiting Time: %d\n", process->pcb.waitingTime);
    fprintf(file, "----------------------------------\n");

    fclose(file); // Close the file after writing
}



//RR_switching --> start running processes
void RR_Switching(Queue* rr_ready_queue, int c, int* quanta){
    int pid;
    //should not happen
    if(is_queue_empty(rr_ready_queue))
    {
        return;
    }
    runningProcess = dequeue(rr_ready_queue);
    // check if it is a new process
    if (runningProcess->processId == NEW_PROCESS)
    {
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrival_time - runningProcess->runtime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = STARTED_STATE;
        runningProcess->pcb.waitingTime = c - runningProcess->arrival_time;
        runningProcess->startTime = c;

        // fork new process
        pid = fork();
        if(pid == ERROR){
            perror("Error while forking\n");
            exit(ERROR);
        }

        if (pid == CHILD_PROCESS)
        {
            // send process to process.c
            execl("./process.out", "process.out", NULL);
        }
        else
        {
            // save the id in the process
            runningProcess->processId = pid;
        }
    }
    else{ //preeamtive logic
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrival_time - runningProcess->runtime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = RESUMED_STATE;
        kill(runningProcess->processId, SIGCONT); //giving a signal to the process to continue execution
    }
    runningProcess->pcb.remainingTime--; //decrease remaining time at context switching
    *quanta = 1;
}


//Round-Robin Algorithm
//Improvements, could send process by ptr to consume less memory
void RR(){
    printf("\nWelcome to RR\n");
    //will help in synchronization (clk will tick automatically each clk cycle)
    int prev_clk = -1;

    //enqueueing encoming process
    Queue* rr_ready_queue = create_queue();

    //initialising quanta
    int quanta = 0;

    //initialising/accessing message queue to recieve processes on it; same queue as that of the scheduler 
    key_id = ftok("keychain", SCHEDULER_Q_KEY);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    //message to recieve on the process from the scheduler
    MessageBuffer RR_msg;

    if(msgq_id == ERROR){
        perror("Error in creating/accessing the message Queue!\n");
        exit(ERROR);
    }

    //loop on all process in queue and run according to the quanta
    while(TRUE_CONDITION){
        int c = getClk();
        
        if(prev_clk != c)
        {
            printf("\n-------------------------Current Timestep:  %d----------------------------------\n", c);
            //we finished running and scheduling all processes
            if(Terminated_Processes == ProcessCount){
                // Cleaning up resources
                free_queue(rr_ready_queue);
                msgctl(msgq_id, IPC_RMID, NULL); // Remove the message queue
                break;
            }

            //recieve process from the message queue
            receiveProcesses(rr_ready_queue);

            //printf("Ready Queue size %d\n", sizeQueue(rr_ready_queue));
            //if ready queue is not empty and currently there is no process running (start the algorithm)
            if(sizeQueue(rr_ready_queue) != EMPTY_READY_Q && runningProcess == NULL){
                quanta = 0;
                RR_Switching(rr_ready_queue, c, &quanta);
                //printf("241\n");
                prev_clk = c;
                continue;
            }

            if(runningProcess != NULL){
                //if process finished before quanta had finished
                if(runningProcess->pcb.remainingTime == 0){
                        runningProcess->pcb.state = FINISHED_STATE;
                        runningProcess->pcb.TurnaroundTime = c - runningProcess->arrival_time;
                        runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / runningProcess->runtime;
                        runningProcess->FinishTime = c;
                        runningProcess->pcb.waitingTime = runningProcess->pcb.TurnaroundTime - runningProcess->runtime;
                        PrintProcessState(runningProcess); // Log process state to the terminal (for debugging)
                        WriteProcessStateToFile(runningProcess); // Log process state to a file
                        kill(runningProcess->processId, SIGKILL);
                        //free(runningProcess);
                        Terminated_Processes++;
                        //printf("251\n");
                        //printf("Running process ID: %d\n", runningProcess->id);
                        RR_Switching(rr_ready_queue, c, &quanta);
                        //printf("Running process ID: %d\n", runningProcess->id);
                        prev_clk = c;
                        continue;
                    }
                //TODO: adjust the metrics quanta, runningtime, waittime, ........ 
                if(quanta >= quantum){
                    //printf("239\n");
                    if(runningProcess->pcb.remainingTime <= 0){
                        runningProcess->pcb.state = FINISHED_STATE;
                        runningProcess->pcb.TurnaroundTime = c - runningProcess->arrival_time;
                        runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / runningProcess->runtime;
                        runningProcess->FinishTime = c;
                        PrintProcessState(runningProcess); // Log process state to a file
                        WriteProcessStateToFile(runningProcess); // Log process state to a file
                        //printf("Running process ID: %d", runningProcess->processId);
                        kill(runningProcess->processId, SIGKILL);
                        //free(runningProcess);
                        Terminated_Processes++;
                        //either switch or change running process to NULL
                        //RR_Switching(rr_ready_queue, c, &quanta);
                    }
                    if(runningProcess->pcb.remainingTime > 0){
                        runningProcess->pcb.state = STOPPED_STATE;
                        kill(runningProcess->processId, SIGSTOP);
                        enqueue(rr_ready_queue, runningProcess);
                    }

                    //printf("Quanta = %d\n", quanta);
                    
                    RR_Switching(rr_ready_queue, c, &quanta);
                    
                    //printf("Running process ID: %d\n", runningProcess->id);
                }
                else{
                    //printf("255\n");
                    //printf("Running process ID: %d", runningProcess->id);  //wrong process running here
                    quanta++; //incremented each clk tic
                    runningProcess->pcb.remainingTime--; //decrement remaining time by one clk cycle
                    //printf("Remaining time = %d for process id = %d", runningProcess->pcb.remainingTime, runningProcess->id);
                    //runningProcess->startTime = c; 
                }
            }
            prev_clk = c; //won't enter the loop if clk was not incremented
        }
    }

}




int main(int argc, char *argv[]) {
    initClk();
    
    int AlgoType = atoi(argv[1]);
    quantum = atoi(argv[2]);
    ProcessCount = atoi(argv[3]);
    
    printf("process count is %d",ProcessCount);

    switch (AlgoType)
    {
    case 1:
            //SJF();
            break;
        case 2:
            //HPF();
            break;
        case 3:
            RR();
            break;
        case 4:
            //Multilevel();

        default:
            printf("invalid algorithm");
            break;
    }
    
    printf("We finished Scheduling and running all of the processes successfully!\n");

    destroyClk(true);

    return 0;
}

