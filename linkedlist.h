#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdint.h>
#include <urcu/urcu-qsbr.h> // For RCU QSBR
#include <stddef.h>

// Node structure
typedef struct Node {
    uint64_t value;
    struct Node *next;
    struct rcu_head rcu_head; // Required for urcu_qsbr_call_rcu()
} Node;

// Linked list structure
typedef struct LinkedList {
    Node *head;
} LinkedList;

// Function prototypes
LinkedList* init_list(void);
void destroy_list(LinkedList *list);
int add_node(LinkedList *list, uint64_t value);
int delete_node(LinkedList *list, uint64_t value);
int contains(LinkedList *list, uint64_t value);

#endif // LINKEDLIST_H
