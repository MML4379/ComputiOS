// ComputiOS Kernel
// Copyright (c) 2025 MML Tech LLC

#include "../apis/libc/libc.h"
#include "cposapi/kernel.hpp"

#define NUM_QUEUES 3

Queue mlfq[NUM_QUEUES]; // Array of queues

// Time quanta for each queue
unsigned long queue_quantum[NUM_QUEUES] = {4, 8, 16};

// Initialize queues
void init_queues() {
    for (int i = 0; i < NUM_QUEUES; i++) {
        mlfq[i].front = 0;
        mlfq[i].rear = 0;
    }
}

/**
 * Adds a process to the queue
 * @param q: The process' current queue level
 * @param process: The process ID you want to add to the queue
 * @returns Nothing
 */
void enqueue(Queue *q, Process *process) {
    q->processes[q->rear++] = process; // Add to end of queue
}

/**
 * Removes a process from the front of the queue
 * @param q: The process current queue level
 * @returns NULL if process is not at the front of the queue OR the new position in the queue
 */
Process *dequeue(Queue *q) {
    if (q->front == q->rear) return NULL;
    return q->processes[q->front++];
}

unsigned long execute_process(Process *process, unsigned long quantum) {
    unsigned long time_used = (process->burst_time - process->cpu_time > quantum)
        ? quantum
        : (process->burst_time - process->cpu_time);
    return time_used;
}

// Scheduler Loop
void schedule() {
    while (1) {
        for (int level = 0; level < NUM_QUEUES; level++) {
            Queue *current_queue = &mlfq[level];

            Process *process = dequeue(current_queue);
            if (process == NULL) continue;

            // Set process quantum based on queue level
            process->quantum = queue_quantum[level];

            // Process execution
            unsigned long time_used = execute_process(process, process->quantum);
            process->cpu_time += time_used;

            // Check if process has finished execution
            if (process->cpu_time >= process->burst_time) continue;

            // Demote if time quantum was fully used
            if (time_used == process->quantum && level < NUM_QUEUES - 1) {
                process->queue_level++;
                enqueue(&mlfq[process->queue_level], process);
            } else {
                enqueue(current_queue, process);
            }
        }
    }
}

void apply_aging() {
    for (int level = 1; level < NUM_QUEUES; level++) {
        Queue *current_queue = &mlfq[level];
        Process *process = dequeue(current_queue);
        for (int i = current_queue->front; i < current_queue->rear; i++) {
            process->queue_level--;
            enqueue(&mlfq[process->queue_level], process);
        }
    }
}