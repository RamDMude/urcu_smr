#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <urcu/urcu-qsbr.h>
#include <unistd.h>

// Helper function to free a stack node using RCU
static void free_stack_node_rcu(struct rcu_head *rcu) {
    StackNode *node = caa_container_of(rcu, StackNode, rcu_head);
    free(node);
}

// Initialize stack
Stack* init_stack() {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    if (!stack) return NULL;
    stack->top = NULL;
    return stack;
}

// Destroy stack
void destroy_stack(Stack *stack) {
    StackNode *curr, *next;
    for (curr = stack->top; curr != NULL; curr = next) {
        next = curr->next;
        free(curr);
    }
    free(stack);
}

// Push an item onto the stack
int push(Stack *stack, uint64_t value) {
    StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
    if (!new_node) return -1;

    new_node->value = value;

    StackNode *old_top = stack->top;
    new_node->next = old_top;
    // stack->top = new_node;
    rcu_assign_pointer(stack->top, new_node);

    return 0;
}

// Pop an item from the stack
int pop(Stack *stack, uint64_t *value) {
    StackNode *old_top = stack->top;

    if (!old_top) {
        return -1; // Stack is empty
    }

    // stack->top = old_top->next;
    rcu_assign_pointer(stack->top, old_top->next);
    *value = old_top->value;

    // urcu_qsbr_call_rcu(&old_top->rcu_head, free_stack_node_rcu);
    printf("Entering delete sync");
    urcu_qsbr_synchronize_rcu();
    printf("Exiting  delete sync");
    free_stack_node_rcu(&old_top->rcu_head);

    return 0;
}

int top(Stack *stack, uint64_t *value) {
    urcu_qsbr_read_lock(); 

    StackNode *curr = rcu_dereference(stack->top);
    if (!curr) { 
        usleep(10*1000);
        urcu_qsbr_read_unlock();
        return -1; 
    }

    *value = curr->value; 
    usleep(10*1000);
    urcu_qsbr_read_unlock(); 

    return 0; 
}
