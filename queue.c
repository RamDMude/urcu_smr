#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <urcu/urcu-qsbr.h>

// Helper function for freeing a node with RCU
static void free_queue_node_rcu(struct rcu_head *rcu) {
    QueueNode *node = caa_container_of(rcu, QueueNode, rcu_head);
    free(node);
}

// Initialize queue
Queue* init_queue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (!queue) return NULL;

    QueueNode *dummy = (QueueNode *)malloc(sizeof(QueueNode));
    if (!dummy) {
        free(queue);
        return NULL;
    }

    dummy->next = NULL;
    queue->head = queue->tail = dummy;
    return queue;
}

// Destroy queue
void destroy_queue(Queue *queue) {
    QueueNode *curr = queue->head;
    while (curr) {
        QueueNode *next = curr->next;
        free(curr);
        curr = next;
    }
    free(queue);
}

// Enqueue a value
int enqueue(Queue *queue, uint64_t value) {
    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!new_node) return -1;

    new_node->value = value;
    new_node->next = NULL;

    urcu_qsbr_read_lock();
    queue->tail->next = new_node;
    queue->tail = new_node;
    urcu_qsbr_read_unlock();

    return 0;
}

// Dequeue a value
int dequeue(Queue *queue, uint64_t *value) {
    urcu_qsbr_read_lock();
    QueueNode *head = queue->head;
    QueueNode *next = head->next;

    if (!next) { // Queue is empty
        urcu_qsbr_read_unlock();
        return -1;
    }

    *value = next->value;
    queue->head = next;

    urcu_qsbr_call_rcu(&head->rcu_head, free_queue_node_rcu);
    urcu_qsbr_read_unlock();

    return 0;
}

int queue_contains(Queue* queue, uint64_t value) {
    QueueNode *head = queue->head;
    while (head != NULL) {
        if (head->value == value) {
            return 1; // Value found in the queue
        }
        head = head->next;
    }
    return 0; // Value not found
}
