#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "structs.h" 

// Define the Queue Node structure
typedef struct QueueNode {
    Process* data;          // Change data to be a pointer to Process
    struct QueueNode* next;
} QueueNode;

// Define the Queue structure
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int size;
} Queue;

// Function to create a new queue
Queue* create_queue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Failed to allocate memory for queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

// Function to check if the queue is empty
bool is_queue_empty(Queue* q) {
    return (q->size == 0);
}

// Function to enqueue a process into the queue
void enqueue(Queue* q, Process* process) {  // Pass a pointer to Process
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!new_node) {
        perror("Failed to allocate memory for queue node");
        exit(EXIT_FAILURE);
    }
    new_node->data = process;  // Store the pointer to Process
    new_node->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = new_node;
    } else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
}

// Function to dequeue a process from the queue
Process* dequeue(Queue* q) {  // Return a pointer to Process
    if (is_queue_empty(q)) {
        fprintf(stderr, "Queue underflow: Attempt to dequeue from an empty queue\n");
        exit(EXIT_FAILURE);
    }

    QueueNode* temp = q->front;
    Process* data = temp->data;  // Get the pointer to Process
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    q->size--;
    return data;  // Return the pointer to Process
}

// Function to check if the queue is empty
int isEmptyQueue(Queue* q) {
    return q->size == 0;
}

// Function to get the size of the queue
int sizeQueue(Queue* q) {
    return q->size;
}

// Function to free the queue memory
void free_queue(Queue* q) {
    while (!is_queue_empty(q)) {
        Process* process = dequeue(q);  // Dequeue a pointer to Process
        free(process);  // Free the memory allocated for the Process
    }
    free(q);
}


Process* peek(Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty.\n");
        return NULL; // Return a sentinel value or handle it appropriately
    }
    return queue->front->data;
}

#endif // QUEUE_H
