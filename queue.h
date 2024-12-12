#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <urcu/urcu-qsbr.h>

typedef struct QueueNode {
    uint64_t value;
    struct QueueNode *next;
    struct rcu_head rcu_head;
} QueueNode;

typedef struct Queue {
    QueueNode *head;
    QueueNode *tail;
} Queue;

Queue* init_queue();
void destroy_queue(Queue *queue);
int enqueue(Queue *queue, uint64_t value);
int dequeue(Queue *queue, uint64_t *value);
int peek(Queue *queue, uint64_t* value);

#endif // QUEUE_H
