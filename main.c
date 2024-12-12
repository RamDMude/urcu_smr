#include "linkedlist.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <urcu/urcu-qsbr.h>
#include <inttypes.h>
#include <sched.h>
#include <time.h>

#define THREAD_COUNT 10 //Determines the total number of threads for the program
#define WRITER_THREADS 1 //Set number of writer threads
#define TOGGLE_PREEMPTION 0 //Set to 1 for Preemption and 0 for Non_Preemption
#define PROGRAM_DURATION 10 //Vary how long the threads run for

//This strcut is used to pass parameters to the thread during thread creatation
typedef struct{
    int priority;
    LinkedList *list;
}LL_Thread_Params;

void* thread_func(void *arg) {
    int priority = 0;
#ifdef USE_LINKED_LIST
    //Seperates the arguments that were passed into the thread_func into there respective variables
    LL_Thread_Params* parameters = (LL_Thread_Params*)arg;
    LinkedList *list = parameters->list;
    priority = parameters->priority;
    
#endif
    urcu_qsbr_register_thread(); //required in the userpsace rcu structure
    printf("PRIORITY %" PRIu64 " \n", priority);
    //Varibles to track how long a thread has been running for
    clock_t start = clock();
    clock_t end = clock();
    while(((double)(end - start)/CLOCKS_PER_SEC) < PROGRAM_DURATION) {
        urcu_qsbr_quiescent_state();
        //Determines the value we are going to search for, add, or attempt to delete
        uint64_t value = rand() % 100;
        if (priority == 1) { // 80% reads
#ifdef USE_LINKED_LIST
            if (contains(list, value)) {
                printf("Value %" PRIu64 " found in linked list.\n", value);
            }
#endif
        } else if(TOGGLE_PREEMPTION) { // 20% writes
            
#ifdef USE_LINKED_LIST
                add_node(list, value);
                printf("Added value %" PRIu64 " to linked list.\n", value);
#endif
            }
        else{
            if (rand() % 2) {
#ifdef USE_LINKED_LIST
                add_node(list, value);
                printf("Added value %" PRIu64 " to linked list.\n", value);
#endif
            } else{
#ifdef USE_LINKED_LIST
                if (delete_node(list, value) == 0) {
                    printf("Deleted value %" PRIu64 " from linked list.\n", value);
                }
#endif
                }
        
        }
        //Tracks the time after each iteration to determine the length of execution
        end = clock();
    }
    urcu_qsbr_unregister_thread(); //Must unregister the thread after thread has finished work within the rcu

    exit(0); //To end execution after one thread finishes in the alloted time.
    //if we do not invoke exit, then when doing preemption the time for the program to finish will take hours/days.
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
#endif

    pthread_t threads[THREAD_COUNT];
#ifdef USE_LINKED_LIST
    LL_Thread_Params parameters[THREAD_COUNT];
#endif
    for (int i = 0; i < THREAD_COUNT; i++) {
#ifdef USE_LINKED_LIST
        parameters[i].list = list;
        // pthread_attr_init(&attr[i]);
        //Seperates the threads into readers and writers (80/20 ratio)
        if(i >WRITER_THREADS){
            parameters[i].priority = 1; //Reader Threads
            
        }else{
            parameters[i].priority = 2; //Writer Threads
            
        }
       
        pthread_create(&threads[i], NULL, thread_func, (void*)&parameters[i]);
        // printf("Thread priority: %d\n", params[i].sched_priority);
#endif
    }


    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

#ifdef USE_LINKED_LIST
    destroy_list(list);
#endif

    // urcu_qsbr_unregister_thread();

    return 0;
}
