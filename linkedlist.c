#include "linkedlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <urcu/urcu-qsbr.h> // For RCU QSBR
#include <unistd.h>
// Helper function for freeing a node with RCU

#define SLEEP_COUNT 1 // Count is in milliseconds. Set to 100000 for preemption and 1 for non-preemption
#define TOGGLE_PREEMPTION 0 //Set to 1 for preemption and 0 for Non-Preemption
static void free_node_rcu(struct rcu_head *rcu) {
    Node *node = caa_container_of(rcu, Node, rcu_head);
    //method call to reclaim the memory of the node
    free(node);
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
    
    //rcu_dereference allows the abiloty to grab data from a pointer within rcu
    Node* old_head = rcu_dereference(list->head);
    //Adds nodes to the list using rcu_assign_pointer function
    rcu_assign_pointer(new_node->next, old_head);
    
    rcu_assign_pointer(list->head, new_node);
    
    if(TOGGLE_PREEMPTION){
        //call_rcu is a non_blocking call. So it will execute even if readers still hold a reference to the data
        urcu_qsbr_call_rcu(&new_node->rcu_head, free_node_rcu);
    }
    
    return 0;
}

// Delete a node from the list
int delete_node(LinkedList *list, uint64_t value) {
    Node *prev = NULL;
    
    Node *curr = rcu_dereference(list->head);

    while (curr) {
        if (curr->value == value) { //Searches for a node, if found then runs the following chunk of code
            if (prev) {
                prev->next = curr->next;
            } else {
                list->head = curr->next;
            }
            
            
            urcu_qsbr_synchronize_rcu();
            free_node_rcu(&curr->rcu_head);
            
            return 0; // Success
        }
        prev = curr;
        curr = curr->next;
    }
    //If the node is not found then we return -1
    return -1;
}

// Check if a value exists in the list
int contains(LinkedList *list, uint64_t value) {
    urcu_qsbr_read_lock(); //Begins a read side critical section
    Node *curr = rcu_dereference(list->head);
    //printf("contains invoked");
    while (curr) {
        if (curr->value == value) { //If the value is found then this chunk of code will run
            //sched_yield();
            usleep(SLEEP_COUNT * 1000);
            urcu_qsbr_read_unlock();
            return 1; // Found
        }
        curr = curr->next;
    }
    usleep(SLEEP_COUNT * 1000);
    urcu_qsbr_read_unlock(); //Ends of read side critical section
    return 0; // Not found
}
