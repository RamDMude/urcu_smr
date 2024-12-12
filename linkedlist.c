#include "linkedlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <urcu/urcu-qsbr.h> // For RCU QSBR
#include <unistd.h>
// Helper function for freeing a node with RCU
static void free_node_rcu(struct rcu_head *rcu) {
    Node *node = caa_container_of(rcu, Node, rcu_head);
    free(node);
    //printf("Free is Called\n");
}

// Initialize linked list
LinkedList* init_list() {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (!list) return NULL;
    list->head = NULL;
    return list;
}

// Destroy linked list
void destroy_list(LinkedList *list) {
    Node *curr, *next;
    for (curr = list->head; curr != NULL; curr = next) {
        next = curr->next;
        free(curr);
    }
    free(list);
}

// Add a node to the list
int add_node(LinkedList *list, uint64_t value) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) return -1;
    
    new_node->value = value;
    
    //urcu_qsbr_read_lock();
    //Node *old_head = list->head;
    Node* old_head = rcu_dereference(list->head);
    //new_node->next = old_head;
    rcu_assign_pointer(new_node->next, old_head);
    //list->head = new_node;
    rcu_assign_pointer(list->head, new_node);
    //urcu_qsbr_read_unlock();
    urcu_qsbr_call_rcu(&new_node->rcu_head, free_node_rcu);
    return 0;
}

// Delete a node from the list
int delete_node(LinkedList *list, uint64_t value) {
    Node *prev = NULL;
    // Node *curr = list->head;
    //urcu_qsbr_read_lock();
    Node *curr = rcu_dereference(list->head);

    while (curr) {
        if (curr->value == value) {
            if (prev) {
                prev->next = curr->next;
            } else {
                list->head = curr->next;
            }
            
            //urcu_qsbr_read_unlock();
            urcu_qsbr_synchronize_rcu();
            free_node_rcu(&curr->rcu_head);
            
            return 0; // Success
        }
        prev = curr;
        curr = curr->next;
    }
    //urcu_qsbr_read_unlock();
    //urcu_qsbr_synchronize_rcu();
    return -1; // Not found
}

// Check if a value exists in the list
int contains(LinkedList *list, uint64_t value) {
    urcu_qsbr_read_lock();
    Node *curr = rcu_dereference(list->head);
    //printf("contains invoked");
    while (curr) {
        if (curr->value == value) {
            //sched_yield();
            usleep(100000 * 1000);
            urcu_qsbr_read_unlock();
            return 1; // Found
        }
        curr = curr->next;
    }
    //sched_yield();
    usleep(100000 * 1000);
    urcu_qsbr_read_unlock();
    return 0; // Not found
}
