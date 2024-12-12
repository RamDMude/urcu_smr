#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <urcu/urcu-qsbr.h> // For RCU QSBR
#include <stddef.h>

// Node structure for the stack
typedef struct StackNode {
    uint64_t value;
    struct StackNode *next;
    struct rcu_head rcu_head; // RCU-specific for safe reclamation
} StackNode;

// Stack structure
typedef struct {
    StackNode *top;
} Stack;

// Function prototypes
Stack* init_stack();
void destroy_stack(Stack *stack);
int push(Stack *stack, uint64_t value);
int pop(Stack *stack, uint64_t *value);
int top(Stack *stack, uint64_t *value);

#endif // STACK_H
