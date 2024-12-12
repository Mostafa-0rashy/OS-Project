#ifndef STRUCTS_H
#define STRUCTS_H
typedef struct Process {
    int id;
    int arrival_time;
    int runtime;
    int priority;
} Process;
typedef struct MessageBuffer
{
    long mtype;
    struct Process process;
}MessageBuffer;

#endif