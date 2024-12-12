#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <urcu/urcu-qsbr.h>

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

    urcu_qsbr_read_lock();
    StackNode *old_top = stack->top;
    new_node->next = old_top;
    stack->top = new_node;
    urcu_qsbr_read_unlock();

    return 0;
}

// Pop an item from the stack
int pop(Stack *stack, uint64_t *value) {
    urcu_qsbr_read_lock();
    StackNode *old_top = stack->top;

    if (!old_top) {
        urcu_qsbr_read_unlock();
        return -1; // Stack is empty
    }

    stack->top = old_top->next;
    *value = old_top->value;

    urcu_qsbr_call_rcu(&old_top->rcu_head, free_stack_node_rcu);
    urcu_qsbr_read_unlock();

    return 0;
}

// Check if a value exists in the stack
int stack_contains(Stack *stack, uint64_t value) {
    urcu_qsbr_read_lock();
    StackNode *curr = stack->top;

    while (curr) {
        if (curr->value == value) {
            urcu_qsbr_read_unlock();
            return 1; // Found
        }
        curr = curr->next;
    }

    urcu_qsbr_read_unlock();
    return 0; // Not found
}
