#include<assert.h>
#ifndef STRUCTS_H
#define STRUCTS_H


typedef struct PCB {
    int state; // 0:started, 1:resumed, 2:stopped, 3:finished
    int remainingTime; //remaining to to finish execution
    int waitingTime;
    int TurnaroundTime;
    double WeightedTurnaroundTime;
    int WaitingtimeSoFar;
} PCB;


typedef struct Process {
    int id;
    int processId;
    int arrival_time;
    int runtime;
    int priority;
    int TimeInProcessor;//Time spent in processor
    int startTime;//Start Time in processor
    int FinishTime;
    int memSize;
    PCB pcb;
} Process;


//Process constructor
struct Process *Create_Process(int id,int at, int rt, int pr,int memory)
{
    struct Process *p = malloc(sizeof(struct Process));
    assert(p != NULL); //error if memory failed to be allocated

    p->id = id;
    p->processId = -1;  //to know whether the process is newly created or not
    p->arrival_time = at;
    p->runtime = rt;
    p->priority = pr;
    p->memSize=memory;
    p->pcb.state=0;
    p->pcb.remainingTime= rt;
    p->pcb.waitingTime=0;
    //p->memsize = ms;  //may need to store the memory size of the process

    return p;
}


//Process destructor
void Destroy_Process(struct Process* p)
{
    assert(p != NULL);

    free(p);
}


typedef struct MessageBuffer
{
    long mtype;
    Process process;
}MessageBuffer;




#endif