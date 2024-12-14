#include "headers.h"

#define PROCESS_GEN_SCHEDULER 6
#define SCHEDULER_Q_KEY 65
#define Error -1
#define EMPTY_MESSAGE_QUEUE -1
#define EMPTY_READY_Q 0
#define TRUE_CONDITION 1
#define PROCESS_KEY 12
#define PROCESS_LOCK 6
#define NEW_PROCESS -1
#define STARTED_STATE 0
#define RESUMED_STATE 1
#define CHILD_PROCESS 0


//global variables
Process* runningProcess = NULL;  //points to current running process
MessageBuffer message; //hold the message received from the message queue of the schedular
int msgq_id; //global variable of message queue (running queue of the scheduling algorithm)




struct Process getProcess(int processcount) {
    struct MessageBuffer message;
    key_t key = ftok("keyFile", SCHEDULER_Q_KEY);
    int msgqid = msgget(key, IPC_CREAT | 0666);

    if (msgqid == -1) {
        perror("Error creating message queue");
        exit(-1);
    }
    
    printf("Attempting to receive message...\n");
    int r = msgrcv(msgqid, &message, sizeof(message.process), PROCESS_GEN_SCHEDULER, !IPC_NOWAIT);
    printf("Scheduler received process with pid %d from message queue at time %d\n", message.process.id, getClk());

    return message.process;
    
}




//RR_switching --> start running processes
void RR_Switching(Queue* rr_ready_queue, MessageBuffer RR_msg, int c){
    int pid;
    runningProcess = dequeue(rr_ready_queue);

    // check if it is a new process
    if (runningProcess->processId == NEW_PROCESS)
    {
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrival_time - runningProcess->runtime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = STARTED_STATE;
        runningProcess->pcb.waitingTime = c - runningProcess->arrival_time;
        runningProcess->startTime = c - runningProcess->arrival_time;

        // fork new process
        pid = fork();
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
        // add the new state to the output file
        //PrintCurrentState(runningProcess);
    }
    else{ //preeamtive logic
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrival_time - runningProcess->runtime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = RESUMED_STATE;
        runningProcess->pcb.waitingTime = c - runningProcess->arrival_time;
        //PrintCurrentState(runningProcess);
        kill(runningProcess->processId, SIGCONT); //giving a signal to the process to continue execution
    }

    // send message queue to process file with the running process
    RR_msg.process = *runningProcess;

    int sen_val = msgsnd(msgq_id, &RR_msg, sizeof(RR_msg.process), !IPC_NOWAIT);  //msgid needs to be a global variable
    if (sen_val == ERROR)
    {
        perror("msgrcv error");
    }
    

}


//Round-Robin Algorithm
//Improvements, could send process by ptr to consume less memory
void RR(){
    //stroing the clk's current value
    int c = getclk();

    //enqueueing encoming process
    Queue* rr_ready_queue = create_queue();

    //initialising quanta
    int quanta = 0;

    //initialising/accessing message queue to recieve processes on it; same queue as that of the scheduler 
    key_t key_id = ftok("keychain", SCHEDULER_Q_KEY);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    //message to recieve on the process from the scheduler
    MessageBuffer RR_msg;
    RR_msg.mtype = PROCESS_LOCK; //PROCESS_LOCK indicates that the message is in the message queue between the schedular and the process

    if(msgq_id == Error){
        perror("Error in creating/accessing the message Queue!\n");
        exit(Error);
    }

    //loop on all process in queue and run according to the quanta
    while(TRUE_CONDITION){
        int msgR = msgrcv(msgq_id, &RR_msg, sizeof(RR_msg.process), PROCESS_GEN_SCHEDULER, IPC_NOWAIT);  //can change to !IPC_NOWAIT later when testing

        //while loop to receive messages and enqueue processes in the ready queue
        while(msgR != EMPTY_MESSAGE_QUEUE){

            //create the new process
            Process* new_process = Create_Process(message.process.id, message.process.arrival_time, message.process.runtime, message.process.priority);
            printf("received process id %d at time %d\n", new_process->id, getClk());

            //enqueueing the received process 
            enqueue(ready_queue, new_process);

            //recieve another process if there are multiple processes arriving at the same time
            msgR = msgrcv(msgq_id, &RR_msg, sizeof(RR_msg.process), PROCESS_GEN_SCHEDULER, IPC_NOWAIT);
        }

        //if ready queue is not empty and currently there is no process running (start the algorithm)
        if(sizeQueue(rr_ready_queue) != EMPTY_READY_Q && runningProcess == NULL){
            quanta = 0;
            RR_Switching(rr_ready_queue, RR_msg, c);
        }

        if(runningProcess != NULL){
            //TODO: adjust the metrics quanta, runningtime, waittime, ........ 
        }

    }

}




int main(int argc, char *argv[]) {
    initClk();
    int AlgoType=atoi(argv[1]);
    int quantum=atoi(argv[2]);
    int processcount=atoi(argv[3]);
    int processin=0;

    //initialising schedular message queue to receive process info from process generator
    key_t key_id = ftok("keyfile", SCHEDULER_Q_KEY);
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == Error){
        perror("Could not create or access Scheduler message queue");
        exit(Error);
    }

    //Recieve a message containing process info from the process generator
    MessageBuffer message;
    int msg_received = msgrcv(msgq_id, &message, sizeof(message.process), PROCESS_GEN_SCHEDULER, !IPC_NOWAIT);

    //No need to valiadate msg_recieved as (!IPC_NOWAIT) will block the call till it receive a valid message in the queue
    

    printf("process count is %d",processcount);
    while (processin<=processcount)
    {
        //process arrived to schedular
        //could add an if conditional to verify if a process arrived or not
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

