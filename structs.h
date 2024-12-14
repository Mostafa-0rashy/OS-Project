#ifndef STRUCTS_H
#define STRUCTS_H
typedef struct Process {
    int id;
    int arrival_time;
    int runtime;
    int priority;
    int TimeInProcessor;//Time spent in processor
    int remTime;//Remaing time till termination
    int startTime;//Start Time in processor
} Process;
typedef struct MessageBuffer
{
    long mtype;
    struct Process process;
}MessageBuffer;

#endif