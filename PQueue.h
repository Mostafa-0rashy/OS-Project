#ifndef PQUEUE_H
#define PQUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "structs.h" // Assuming this is where Process is defined.

// Define the Priority Queue Node structure
typedef struct PQueueNode
{
    Process *data; // Pointer to a Process
    struct PQueueNode *next;
} PQueueNode;

// Define the Priority Queue structure
typedef struct
{
    PQueueNode *front;
    PQueueNode *rear;
    int size;
} PQueue;

// Function to create a new priority queue
PQueue *pcreate_queue()
{
    PQueue *q = (PQueue *)malloc(sizeof(PQueue));
    if (!q)
    {
        perror("Failed to allocate memory for priority queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

// Function to check if the priority queue is empty
bool pis_queue_empty(PQueue *q)
{
    return (q->size == 0);
}

// Function to enqueue a process into the priority queue
void penqueue(PQueue *pq, Process *process, int SJFflag)
{ // Accept pointer to Process
    PQueueNode *new_node = (PQueueNode *)malloc(sizeof(PQueueNode));
    if (!new_node)
    {
        perror("Failed to allocate memory for priority queue node");
        exit(EXIT_FAILURE);
    }
    new_node->data = process; // Store the pointer to Process
    new_node->next = NULL;

    // Check SJFflag and enqueue accordingly
    if (SJFflag == 1)
    {
        // Shortest Job First: Sort by runtime (ascending order)
        if (pq->front == NULL || pq->front->data->runtime >= process->runtime)
        {
            new_node->next = pq->front;
            pq->front = new_node;
        }
        else
        {
            PQueueNode *current = pq->front;
            while (current->next != NULL && current->next->data->runtime < process->runtime)
            {
                current = current->next;
            }
            new_node->next = current->next;
            current->next = new_node;
        }
    }
    else
    {
        // General Priority: Sort by priority (ascending order)
        if (pq->front == NULL || pq->front->data->priority >= process->priority)
        {
            new_node->next = pq->front;
            pq->front = new_node;
        }
        else
        {
            PQueueNode *current = pq->front;
            while (current->next != NULL && current->next->data->priority < process->priority)
            {
                current = current->next;
            }
            new_node->next = current->next;
            current->next = new_node;
        }
    }

    pq->size++;
}

// Function to dequeue a process from the priority queue
Process *pdequeue(PQueue *q)
{ // Return pointer to Process
    if (pis_queue_empty(q))
    {
        fprintf(stderr, "Queue underflow: Attempt to dequeue from an empty priority queue\n");
        exit(EXIT_FAILURE);
    }

    PQueueNode *temp = q->front;
    Process *data = temp->data; // Get the pointer to Process
    q->front = q->front->next;

    if (q->front == NULL)
    {
        q->rear = NULL;
    }

    free(temp);
    q->size--;
    return data; // Return pointer to Process
}

// Function to get the size of the priority queue
int psizeQueue(PQueue *q)
{
    return q->size;
}

// Function to free the priority queue memory
void pfree_queue(PQueue *q)
{
    while (!pis_queue_empty(q))
    {
        Process *process = pdequeue(q); // Dequeue a pointer to Process
        free(process);                  // Free the memory allocated for the Process
    }
    free(q);
}
Process* PPeek(Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty.\n");
        return NULL; // Return a sentinel value or handle it appropriately
    }
    return queue->front->data;
}




#endif // PQUEUE_H