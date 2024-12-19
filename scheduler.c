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
#define MAX_PRIORITY 12


//global variables
Process* runningProcess = NULL;  //points to current running process
MessageBuffer message; //hold the message received from the message queue of the schedular
int msgq_id; //global variable of message queue (running queue of the scheduling algorithm)
int key_id;
int quantum;
int Terminated_Processes = 0; //counts the number of terminated processes
int ProcessCount; //Stores the total number of processes to be scheduled
int idle_count;
float total_wait_time = 0;
int total_weighted_turnaround_time = 0;




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


void preceiveProcesses(PQueue* ready_queue) {
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
        penqueue(ready_queue, new_process,1);
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



void WritingAverages(float cpu_util, float Average_Wait_Time, float Average_weighted_turnaround_time) {
    FILE* file = fopen("scheduler_perf.txt", "w"); // Open the file in append mode
    if (file == NULL) {
        perror("Failed to open scheduler_out.txt");
        return;
    }
    
    // Writing averages to the file
    fprintf(file, "----------------------------------\n");
    fprintf(file, "CPU Utilization = %.2f%%\n", cpu_util); // Added %% for clarity
    fprintf(file, "Average Wait Time = %.2f\n", Average_Wait_Time);
    fprintf(file, "Average Weighted Turnaround Time = %.2f\n", Average_weighted_turnaround_time);
    fprintf(file, "----------------------------------\n");

    fclose(file); // Always close the file
}


void WriteProcessStateToFile(Process* process) {
    FILE* file = fopen("scheduler_log.txt", "a"); // Open the file in append mode
    if (file == NULL) {
        perror("Failed to open scheduler_out.txt");
        return;
    }

    /*
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
    */
    if(process->pcb.remainingTime == 0 && GetStateName(process->pcb.state) == "FINISHED"){
        fprintf(file,"At time %d  process %d  %s  arr %d  total %d  remain %d  wait %d  TA %d  WTA %f\n",getClk(), process->id, GetStateName(process->pcb.state), process->arrival_time, process->runtime, process->pcb.remainingTime, process->pcb.waitingTime, process->pcb.TurnaroundTime, process->pcb.WeightedTurnaroundTime);
    }
    else{
        fprintf(file,"At time %d  process %d  %s  arr %d  total %d  remain %d  wait %d\n",getClk(), process->id, GetStateName(process->pcb.state), process->arrival_time, process->runtime, process->pcb.remainingTime, process->pcb.waitingTime);
    }
    

    fclose(file); // Close the file after writing
}



//RR_switching --> start running processes
void RR_Switching(Queue* rr_ready_queue, int c, int* quanta){
    int pid;
    //should not happen
    if(is_queue_empty(rr_ready_queue))
    {
        runningProcess = NULL;
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

            //counting idle clk cycles
            
            if(sizeQueue(rr_ready_queue) == EMPTY_READY_Q && runningProcess == NULL){
                idle_count++;
                prev_clk = c;
                continue;
            }
            

            //printf("Ready Queue size %d\n", sizeQueue(rr_ready_queue));
            //if ready queue is not empty and currently there is no process running (start the algorithm)
            if(sizeQueue(rr_ready_queue) != EMPTY_READY_Q && runningProcess == NULL){
                quanta = 0;
                RR_Switching(rr_ready_queue, c, &quanta);
                WriteProcessStateToFile(runningProcess);
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
                        total_wait_time += runningProcess->pcb.waitingTime;
                        total_weighted_turnaround_time += runningProcess->pcb.WeightedTurnaroundTime;
                        PrintProcessState(runningProcess); // Log process state to the terminal (for debugging)
                        WriteProcessStateToFile(runningProcess); // Log process state to a file
                        kill(runningProcess->processId, SIGKILL);
                        //free(runningProcess);
                        Terminated_Processes++;
                        //printf("251\n");
                        //printf("Running process ID: %d\n", runningProcess->id);
                        RR_Switching(rr_ready_queue, c, &quanta);
                        //WriteProcessStateToFile(runningProcess);
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
                    WriteProcessStateToFile(runningProcess);
                    //printf("Running process ID: %d\n", runningProcess->id);
                }
                else{
                    //printf("255\n");
                    //printf("Running process ID: %d", runningProcess->id);  //wrong process running here
                    quanta++; //incremented each clk tic
                    runningProcess->pcb.remainingTime--; //decrement remaining time by one clk cycle
                    WriteProcessStateToFile(runningProcess);
                    //printf("Remaining time = %d for process id = %d", runningProcess->pcb.remainingTime, runningProcess->id);
                    //runningProcess->startTime = c; 
                }
            }
            prev_clk = c; //won't enter the loop if clk was not incremented
        }
    }

}





void SJF()
{
    printf("\nWelcome to Non-Preemptive SJF\n");

    // Initial clock value
    int prev_clk = -1;

    // Create a priority queue for SJF
    PQueue *sjf_ready_queue = pcreate_queue();

    // Initialize message queue
    key_id = ftok("keychain", SCHEDULER_Q_KEY);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == ERROR)
    {
        perror("Error in creating/accessing the message Queue!\n");
        exit(ERROR);
    }

    // Loop to manage processes and perform SJF scheduling
    while (TRUE_CONDITION)
    {
        int c = getClk(); // Get the current clock cycle

        if (prev_clk != c)
        {
            printf("\n-------------------------Current Timestep:  %d----------------------------------\n", c);

            // If all processes have finished, exit the loop and clean up
            if (Terminated_Processes == ProcessCount)
            {
                // Cleanup resources
                pfree_queue(sjf_ready_queue);
                msgctl(msgq_id, IPC_RMID, NULL); // Remove the message queue
                break;
            }

            // Receive processes from the message queue
            preceiveProcesses(sjf_ready_queue);

            // If the ready queue is not empty and no process is running, start a new process
            if (psizeQueue(sjf_ready_queue) != EMPTY_READY_Q && runningProcess == NULL)
            {
                // Get the process with the shortest runtime from the queue
                runningProcess = pdequeue(sjf_ready_queue); // Non-preemptive: pick the shortest job
                printf("edeena haga1");
                runningProcess->pcb.state = STARTED_STATE;
                runningProcess->startTime = c;

                int pid = fork();
                if (pid == ERROR)
                {
                    perror("Error while forking\n");
                    exit(ERROR);
                }

                if (pid == CHILD_PROCESS)
                {
                    // Child process executes the process
                    execl("./process.out", "process.out", NULL);
                }
                else
                {
                    // Parent process saves the process ID
                    runningProcess->processId = pid;
                }
            }

            // If a process is running, check if it finished
            if (runningProcess != NULL)
            {
                // runningProcess->pcb.remainingTime--;
                printf("remainnnnnnnnnn time %d", runningProcess->pcb.remainingTime);
                // If process has finished
                if (runningProcess->pcb.remainingTime == 0)
                {
                    runningProcess->pcb.state = FINISHED_STATE;
                    runningProcess->pcb.TurnaroundTime = c - runningProcess->arrival_time;
                    runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / runningProcess->runtime;
                    runningProcess->FinishTime = c;
                    runningProcess->pcb.waitingTime = runningProcess->pcb.TurnaroundTime - runningProcess->runtime;
                    total_wait_time += runningProcess->pcb.waitingTime;
                    total_weighted_turnaround_time += runningProcess->pcb.WeightedTurnaroundTime;

                    // Log the process state to file
                    PrintProcessState(runningProcess);
                    WriteProcessStateToFile(runningProcess);

                    // Terminate the process and clean up
                    kill(runningProcess->processId, SIGKILL);
                    free(runningProcess); // Free memory after process termination
                    Terminated_Processes++;
                    
                    printf("ahla mesa");

                    if(Terminated_Processes == ProcessCount){
                        break;
                    }

                    // After finishing the current process, start the next one (if any)
                    if (psizeQueue(sjf_ready_queue) > 0)
                    {
                        runningProcess = pdequeue(sjf_ready_queue); // Get the next shortest job
                        printf("edena haga baa");
                        runningProcess->pcb.state = STARTED_STATE;
                        runningProcess->startTime = c;

                        // Fork the next process
                        int pid = fork();
                        if (pid == ERROR)
                        {
                            perror("Error while forking\n");
                            exit(ERROR);
                        }

                        if (pid == CHILD_PROCESS)
                        {
                            // Execute the next process
                            execl("./process.out", "process.out", NULL);
                        }
                        else
                        {
                            // Parent process saves the process ID
                            runningProcess->processId = pid;
                        }
                    }
                    runningProcess->pcb.remainingTime--;
                    WriteProcessStateToFile(runningProcess);
                }
                else
                {
                    runningProcess->pcb.remainingTime--;
                    WriteProcessStateToFile(runningProcess);
                }
            }

            prev_clk = c; // Update the clock value
        }
    }
}

void HPF_Switching(PQueue *hpf_ready_queue, int c)
{
    // debugging to ensure finished processes are freed
    if (runningProcess != NULL && runningProcess->pcb.remainingTime <= 0)
    {
        printf("[DEBUG] Process %d has finished. Cleaning up resources.\n", runningProcess->id);

        free(runningProcess);
        runningProcess = NULL; // Reset the pointer
    }

    // If the ready queue is empty, return
    if (pis_queue_empty(hpf_ready_queue))
        return;

    // dequeue the process in the hpf_ready_queue with lowest priority number
    runningProcess = pdequeue(hpf_ready_queue);
    runningProcess->startTime = c;
    runningProcess->pcb.waitingTime = c - runningProcess->arrival_time;
    // If the process is new
    if (runningProcess->pcb.state == NEW_PROCESS)
    {
        // Update waiting time and set start time
        runningProcess->pcb.waitingTime = runningProcess->pcb.TurnaroundTime - runningProcess->runtime;
        runningProcess->startTime = c;
        // to ensure start time is set as there was an error where it was always zero
        if (runningProcess->startTime == 0)
        {
            runningProcess->startTime = c;
        }

        runningProcess->pcb.state = STARTED_STATE;

        printf("[Time %d] Starting Process %d: WaitingTime=%d\n", c, runningProcess->id, runningProcess->pcb.waitingTime);

        // Fork the new process
        int pid = fork();
        if (pid == ERROR)
        {
            perror("Error while forking\n");
            exit(ERROR);
        }

        if (pid == CHILD_PROCESS)
        {
            execl("./process.out", "process.out", NULL);
        }
        else
        {
            runningProcess->processId = pid;
        }
    }
    else
    {
        // Resume a preempted process that is not new

        runningProcess->pcb.state = RESUMED_STATE;
        printf("[Time %d] Resuming Process %d\n", c, runningProcess->id);
        kill(runningProcess->processId, SIGCONT); // Continue the process
    }

    // Decrement remaining time since the process is running
    runningProcess->pcb.remainingTime--;
    WriteProcessStateToFile(runningProcess);
}

void HPF()
{
    printf("\nWelcome to HPF\n");

    int prev_clk = -1;

    // Create a queue for new processes and a priority queue for scheduling
    Queue *temp_queue = create_queue();
    PQueue *hpf_ready_queue = pcreate_queue();

    // Initialize the message queue
    key_id = ftok("keychain", SCHEDULER_Q_KEY);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == ERROR)
    {
        perror("Error in creating/accessing the message Queue!\n");
        exit(ERROR);
    }

    while (TRUE_CONDITION)
    {
        int c = getClk();

        if (prev_clk != c)
        {
            printf("\n-------------------------Current Timestep:  %d----------------------------------\n", c);

            // End condition: All processes have been terminated
            if (Terminated_Processes == ProcessCount)
            {
                // Cleanup resources
                free_queue(temp_queue);
                pfree_queue(hpf_ready_queue);
                msgctl(msgq_id, IPC_RMID, NULL); // Remove the message queue

                break;
            }

            // Receive new processes
            receiveProcesses(temp_queue);

            // Transfer processes from the temporary queue to the priority queue
            while (!is_queue_empty(temp_queue))
            {
                Process *process = dequeue(temp_queue);
                printf("[Time %d] New Process Received: ID=%d, Priority=%d, Arrival=%d\n",
                       c, process->id, process->priority, process->arrival_time);
                penqueue(hpf_ready_queue, process, 0); // Priority-based enqueue
            }

            // Start or continue the next process
            if (runningProcess == NULL && !pis_queue_empty(hpf_ready_queue))
            {
                HPF_Switching(hpf_ready_queue, c);
                WriteProcessStateToFile(runningProcess);
                prev_clk = c;
                continue;
            }

            // check if readyqueue has a process with a higher priority tham the running process
            if (runningProcess != NULL && !pis_queue_empty(hpf_ready_queue))
            {
                Process *next_process = hpf_ready_queue->front->data;
                if (next_process->priority < runningProcess->priority)
                {
                    printf("[Time %d] Preempting Process %d for Process %d\n", c, runningProcess->id, next_process->id);

                    runningProcess->pcb.state = STOPPED_STATE;
                    kill(runningProcess->processId, SIGSTOP); // Stop the running process

                    penqueue(hpf_ready_queue, runningProcess, 0); // Reinsert the current process into the queue
                    HPF_Switching(hpf_ready_queue, c);
                    WriteProcessStateToFile(runningProcess);
                    prev_clk = c;
                    continue;
                }
            }

            if (runningProcess != NULL)
            {
                printf("[Time %d] Running Process %d: RemainingTime=%d\n", c, runningProcess->id, runningProcess->pcb.remainingTime);

                // If the running process finishes execution

                if (runningProcess->pcb.remainingTime <= 0)
                {
                    runningProcess->pcb.state = FINISHED_STATE;
                    runningProcess->pcb.TurnaroundTime = c - runningProcess->arrival_time;
                    runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / runningProcess->runtime;
                    runningProcess->FinishTime = c;
                    runningProcess->pcb.waitingTime = runningProcess->pcb.TurnaroundTime - runningProcess->runtime;
                    PrintProcessState(runningProcess);
                    WriteProcessStateToFile(runningProcess);
                    total_wait_time += runningProcess->pcb.waitingTime;
                    total_weighted_turnaround_time += runningProcess->pcb.WeightedTurnaroundTime;
                    if (runningProcess->processId > 0)
                    {
                        kill(runningProcess->processId, SIGKILL); // Safely terminate??
                    }
                    free(runningProcess);
                    runningProcess = NULL; // to avoid accessing invalid pointer after deallocating the memory
                    Terminated_Processes++;
                    HPF_Switching(hpf_ready_queue, c);
                    WriteProcessStateToFile(runningProcess);
                    prev_clk = c; // to avoid decrementing the remaining time twice in one timestep
                    continue;
                }
                else
                {
                    runningProcess->pcb.remainingTime--;
                    WriteProcessStateToFile(runningProcess);
                }
            }

            prev_clk = c; // Update the previous clock
        }
    }
    destroyClk(true);
}












////////////////////////////////MLFQ/////////////////////////////////
// Initialize MLFQ queues
void initMLFQ(Queue** priorityQueues) {   
    for (int i = 0; i < MAX_PRIORITY; i++) {
       priorityQueues[i]=create_queue(); 
    }
}

// Enqueue the process into the appropriate priority queue
void EnqueueInLevel(Process *newProcess, int priority, Queue** priorityQueues) {
    switch (priority) {
        case 0: enqueue(priorityQueues[0], newProcess); break;
        case 1: enqueue(priorityQueues[1], newProcess); break;
        case 2: enqueue(priorityQueues[2], newProcess); break;
        case 3: enqueue(priorityQueues[3], newProcess); break;
        case 4: enqueue(priorityQueues[4], newProcess); break;
        case 5: enqueue(priorityQueues[5], newProcess); break;
        case 6: enqueue(priorityQueues[6], newProcess); break;
        case 7: enqueue(priorityQueues[7], newProcess); break;
        case 8: enqueue(priorityQueues[8], newProcess); break;
        case 9: enqueue(priorityQueues[9], newProcess); break;
        case 10: enqueue(priorityQueues[10], newProcess); break;
        default: break;
    }
    printf("Process with ID %d and priority %d entered queue with level %d\n", newProcess->id, newProcess->priority, priority);
}

// Function to handle process demotion (if needed)
void demoteProcess(Process *process, Queue** priorityQueues, int crntpriority) {
    printf("\nstart demoting\n");
    if (crntpriority < MAX_PRIORITY - 1) {
        enqueue(priorityQueues[crntpriority + 1], process);
        printf("\n Process %d is demoted to queue %d \n",process->id,crntpriority + 1);
    }
    else
    {//enqueue back in queue 10
        enqueue(priorityQueues[10], process);
        printf("Process already in %dth queue\n",crntpriority);
    }
}

// Function to schedule the next process
void MLFQ_Switching(Queue* priorityQueue, int quanta,int c,int priority,Queue ** priorityQueues) {
    // Loop over all priority levels (from highest to lowest)
             if (is_queue_empty(priorityQueue)) 
                 {  
                        printf("Queue number %d is empty returning to MLFQ\n",priority);
                         return;
                     }
        //run each process in each queue and demote it
      
            runningProcess = dequeue(priorityQueue);

            // Check if the process arrived at the current clock tick
    if (runningProcess->arrival_time == c) {
        int prevclk2=getClk();
        int currentclk2=getClk();
        //busy wait to run the process after its arrival time by one time step
       while(prevclk2==currentclk2){
        currentclk2=getClk();
       }
       printf("Arrived at %d should start running at %d ",runningProcess->arrival_time,getClk());
    }



             int startClk = getClk();  // Starting time of this process
            int endClk = startClk + quanta;  // The time when this quantum will end
            // printf("\n END CLK IS %d\n",endClk);
            if(runningProcess->processId == NEW_PROCESS)//fork and start it
            {
                runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrival_time - runningProcess->runtime + runningProcess->pcb.remainingTime;
                runningProcess->pcb.state = STARTED_STATE;
                runningProcess->pcb.waitingTime = c - runningProcess->arrival_time;
                runningProcess->startTime = c; 
                //fork new process
            int pid = fork();
                if (pid == ERROR) {
                    perror("Error while forking\n");
                    exit(ERROR);
                }
                else{
                      // Parent process: save child PID and mark as started
                    
                 if (pid == CHILD_PROCESS) {
                    // Run the process logic in a separate process (child process)
                        printf("\n\nRunning Process with pid %d at %d\n\n",runningProcess->processId,getClk());
                        execl("./process.out", "process.out", NULL);
                } 
                else {                     
                    usleep(2 * 1000);
                    // Parent process: Save the PID of the running process
                    runningProcess->processId = pid;
                    runningProcess->startTime = startClk;
                    runningProcess->pcb.state = STARTED_STATE;
                }
                
                }
            }
            else{     //Running process isnt a new one so resumes
                printf("Process already exits... Resuming");
                runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrival_time - runningProcess->runtime + runningProcess->pcb.remainingTime;
                runningProcess->pcb.state = RESUMED_STATE;
                kill(runningProcess->processId, SIGCONT); //giving a signal to the process to continue execution
                }

            // Run the process for the duration of the quantum or until it finishes
            int prevClk=-1;
            while (true) 
            {            
                 int currentClk = getClk();  // Get the current clock time 
                if(endClk==currentClk)
                {
                    break;
                }
               if(prevClk != currentClk)
               {
                
                    printf("\n--------------CURRENT CLK IN MLFQ SCHDUELINGG %d-------------------\n",currentClk);
                    prevClk = currentClk; 
                    if (runningProcess->pcb.remainingTime > 0) 
                    {
                        runningProcess->pcb.remainingTime--;
                        printf("\nREM TIME FOR PROCESS %d is %d\n",runningProcess->id,runningProcess->pcb.remainingTime);
                    }           
                if (runningProcess->pcb.remainingTime == 0) {
                printf("Process ID %d has finished execution at time %d\n", runningProcess->id, getClk());
                runningProcess->pcb.state = FINISHED_STATE;
                runningProcess->pcb.TurnaroundTime = c - runningProcess->arrival_time;
                runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / runningProcess->runtime;
                runningProcess->FinishTime = c;
                runningProcess->pcb.waitingTime = runningProcess->pcb.TurnaroundTime - runningProcess->runtime;
                Terminated_Processes++;
                kill(runningProcess->processId, SIGKILL);  // Terminate the process
                PrintProcessState(runningProcess);
                free(runningProcess);
                return;  // Exit as the process has finished
            }

           
            }
    }
             // If the process has remaining time after the quantum, stop and demote it
            printf("\n Process ID %d quantum finished, remaining time: %d\n", runningProcess->id, runningProcess->pcb.remainingTime);
            kill(runningProcess->processId, SIGSTOP);  // Preempt the process
            runningProcess->pcb.state = STOPPED_STATE;
            demoteProcess(runningProcess,priorityQueues,priority);
            runningProcess=NULL;//nth is running
            
    
    printf("CPU is idle.\n");  // If no processes are available in any queue
}


// Main MLFQ function
void MLFQ(int quanta, int processcount) {
    printf("\nWELCOME TO MLFQ\n");
    int c = getClk();  // Get the current clock time
    Queue* priorityQueues[MAX_PRIORITY];  // Array of priority queues
    Queue* RDYQUEUE = create_queue();  // Ready queue to temporarily hold incoming processes
    initMLFQ(priorityQueues);  // Initialize the priority queues
    
    int prevClk = -1;  // Store the previous clock tick
    
    while (Terminated_Processes < processcount) {  // Run until all processes are terminated
        int currentClk = getClk();  // Get the current clock time

        if (currentClk != prevClk) {
            printf("\n------------------Current CLK in MLFQ: %d------------------\n", currentClk);
            prevClk = currentClk;  // Update the clock tick
            //we finished running and scheduling all processes
            if(Terminated_Processes == ProcessCount){
                // Cleaning up resources
                free_queue(RDYQUEUE);
                msgctl(msgq_id, IPC_RMID, NULL); // Remove the message queue
                break;
            }
            printf("At Time %d: Checking for new messages...\n", currentClk);
            receiveProcesses(RDYQUEUE);  // Receive processes and add them to the ready queue

            // If there are processes in the RDYQUEUE, add them to the appropriate priority queue
            if (!is_queue_empty(RDYQUEUE)) {
                Process* arrivedProcess = dequeue(RDYQUEUE);
                EnqueueInLevel(arrivedProcess, arrivedProcess->priority, priorityQueues);
            }
            int processesPresent=0;
            for (int i = 0; i < MAX_PRIORITY; i++)
            {
                if(!is_queue_empty(priorityQueues[i]))
                {
                    processesPresent=processesPresent+sizeQueue(priorityQueues[i]);
                }
            }
                //If all processes are in priority queue [10]
                printf("\n--Process Present in system:%d--\n",processesPresent);
                printf("\n--Size of Priority Queue 11: %d--\n",sizeQueue(priorityQueues[11]));
                if(sizeQueue(priorityQueues[11])==processesPresent)
                {
                    for (int i = 0; i < processesPresent; i++)
                    {
                        Process*P=dequeue(priorityQueues[11]);
                        EnqueueInLevel(P,P->priority,priorityQueues);
                        printf("\n-----Repositioned Process %d in its %d queue----\n",P->id,P->priority);
                    }
                    
                }
           


                if(processesPresent>0){
                 // Check all queues after each clk tick and assign running process
                 for (int i = 0; i < MAX_PRIORITY; i++) //start from 0 highest priority to 10 Lowest priority
                 {       printf("\nChecking queue number %d\n",i);
                            if (!is_queue_empty(priorityQueues[i]))
                             {
                                MLFQ_Switching(priorityQueues[i], quanta, currentClk, i, priorityQueues);
                                break;
                            }
                }
            }

           
        }
    }

    printf("All processes have been terminated. Scheduler exiting.\n");
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
            SJF();
            break;
        case 2:
            HPF();
            break;
        case 3:
            RR();
            break;
        case 4:
            MLFQ(quantum,ProcessCount);

        default:
            printf("invalid algorithm");
            break;
    }
    
    int final_clk = getClk();
    float cpu_util = ((float)(final_clk - idle_count) / final_clk) * 100;
    float Average_Wait_Time = total_wait_time / ProcessCount;
    float Average_weighted_turnaround_time = ((float)total_weighted_turnaround_time) / ProcessCount;
    printf("CPU Utilization = %.2f\n", cpu_util);
    printf("Average Wait Time = %.2f\n", Average_Wait_Time);
    printf("Average Weighted Turnaround Time = %.2f\n", Average_weighted_turnaround_time);

    WritingAverages(cpu_util, Average_Wait_Time, Average_weighted_turnaround_time);

    printf("We finished Scheduling and running all of the processes successfully!\n");

    destroyClk(true);

    return 0;
}

