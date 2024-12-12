#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <urcu/urcu-qsbr.h>
#include <unistd.h>

// Helper function for freeing a node with RCU
static void free_queue_node_rcu(struct rcu_head *rcu) {
    QueueNode *node = caa_container_of(rcu, QueueNode, rcu_head);
    free(node);
    printf("Freed a node");
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

    // queue->tail->next = new_node;
    rcu_assign_pointer(queue->tail->next, new_node);
    // queue->tail = new_node;
    rcu_assign_pointer(queue->tail, new_node);

    return 0;
}

// Dequeue a value
int dequeue(Queue *queue, uint64_t *value) {
    QueueNode *head = queue->head;
    QueueNode *next = head->next;

    if (!next) { // Queue is empty
        return -1;
    }

    *value = next->value;
    // queue->head = next;
    rcu_assign_pointer(queue->head, next);

    // urcu_qsbr_call_rcu(&head->rcu_head, free_queue_node_rcu);
    printf("Entering delete sync");
    urcu_qsbr_synchronize_rcu();
    printf("Exiting  delete sync");
    free_queue_node_rcu(&head->rcu_head);

    return 0;
}

int peek(Queue *queue, uint64_t* value) {
    urcu_qsbr_read_lock(); 

    QueueNode *head = rcu_dereference(queue->head);
    QueueNode *next = rcu_dereference(head->next);

    if (next == NULL) { 
        // usleep(10 *1000);
        urcu_qsbr_read_unlock();
        return -1; // Indicate the queue is empty
    }

    *value = next->value; 
    // usleep(10 *1000);
    urcu_qsbr_read_unlock(); 

    return 0; // Successfully retrieved the value
}
