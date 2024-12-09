#include "linkedlist.h"
#include "stack.h"
#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <urcu/urcu-qsbr.h>
#include <inttypes.h>
#include <sched.h>

#define THREAD_COUNT 10

typedef struct{
    int priority;
    LinkedList *list;
}LL_Thread_Params;

void* thread_func(void *arg) {
#ifdef USE_LINKED_LIST
    LL_Thread_Params* parameters = (LL_Thread_Params*)arg;
    LinkedList *list = parameters->list;
    int priority = parameters->priority;
#elif defined(USE_STACK)
    Stack *stack = (Stack *)arg;
#elif defined(USE_QUEUE)
    Queue *queue = (Queue *)arg;
#endif
    urcu_qsbr_register_thread();

    for (int i = 0; i < 1000000; i++) {
        //printf("Entering quiescent state function %d\n", i);
        urcu_qsbr_quiescent_state();
        //printf("Exiting quiescent state function %d\n", i);
        uint64_t value = rand() % 100;
        //printf("Priority: %" PRIu64 "\n", priority);
        if (priority == 1) { // 80% reads
#ifdef USE_LINKED_LIST
            if (contains(list, value)) {
                //printf("Value %" PRIu64 " found in linked list.\n", value);
            }
#elif defined(USE_STACK)
            if (stack_contains(stack, value)) {
                printf("Value %" PRIu64 " found in stack.\n", value);
            }
#elif defined(USE_QUEUE)
            if (queue_contains(queue, value)) {
                // printf("Value %" PRIu64 " found in queue.\n", value);
            }
#endif
        } else { // 20% writes
            if (rand() % 2) {
#ifdef USE_LINKED_LIST
                add_node(list, value);
                //printf("Added value %" PRIu64 " to linked list.\n", value);
#elif defined(USE_STACK)
                push(stack, value);
                printf("Pushed value %" PRIu64 " onto stack.\n", value);
#elif defined(USE_QUEUE)
                enqueue(queue, value);
                // printf("Enqueued value %" PRIu64 " to queue.\n", value);
#endif
            } else {
#ifdef USE_LINKED_LIST
                if (delete_node(list, value) == 0) {
                    // printf("Deleted value %" PRIu64 " from linked list.\n", value);
                }
#elif defined(USE_STACK)
                uint64_t popped_value;
                if (pop(stack, &popped_value) == 0) {
                    printf("Popped value %" PRIu64 " from stack.\n", popped_value);
                }
#elif defined(USE_QUEUE)
                uint64_t dequeued_value;
                if (dequeue(queue, &dequeued_value) == 0) {
                    // printf("Dequeued value %" PRIu64 " from queue.\n", dequeued_value);
                }
#endif
            }
        }
        
    }
    urcu_qsbr_unregister_thread();
    return NULL;
}

int main() {
    srand(time(NULL));

#ifdef USE_LINKED_LIST
    LinkedList *list = init_list();
    if (!list) {
        fprintf(stderr, "Failed to initialize the linked list.\n");
        return EXIT_FAILURE;
    }
#elif defined(USE_STACK)
    Stack *stack = init_stack();
    if (!stack) {
        fprintf(stderr, "Failed to initialize the stack.\n");
        return EXIT_FAILURE;
    }
#elif defined(USE_QUEUE)
    Queue *queue = init_queue();
    if (!queue) {
        fprintf(stderr, "Failed to initialize the queue.\n");
        return EXIT_FAILURE;
    }
#endif
    // rcu_init();
    // urcu_qsbr_register_thread();
    struct sched_param params[THREAD_COUNT];
    pthread_attr_t attr[THREAD_COUNT];
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
#ifdef USE_LINKED_LIST
        LL_Thread_Params parameters = {0, list};
        pthread_attr_init(&attr[i]);

        if(i > 0.2 * THREAD_COUNT){
            parameters.priority = 1;
            params[i].sched_priority = 1;
        }else{
            parameters.priority = 4;
            params[i].sched_priority = 4;
        }
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
        pthread_attr_setschedparam(&attr[i], &params[i]);
        pthread_create(&threads[i], &attr[i], thread_func, (void*)&parameters);
        printf("Thread priority: %d\n", params[i].sched_priority);
#elif defined(USE_STACK)
        pthread_create(&threads[i], NULL, thread_func, stack);
#elif defined(USE_QUEUE)
        pthread_create(&threads[i], NULL, thread_func, queue);
#endif
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

#ifdef USE_LINKED_LIST
    destroy_list(list);
#elif defined(USE_STACK)
    destroy_stack(stack);
#elif defined(USE_QUEUE)
    destroy_queue(queue);
#endif

    // urcu_qsbr_unregister_thread();

    return 0;
}
