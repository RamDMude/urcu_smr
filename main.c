#include "linkedlist.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <urcu/urcu-qsbr.h>
#include <inttypes.h>

void* thread_func(void *arg) {
    LinkedList *list = (LinkedList *)arg;

    for (int i = 0; i < 100; i++) {
        if (rand() % 100 < 80) { // 80% reads
            uint64_t value = rand() % 100;
            if (contains(list, value)) {
                printf("Value %" PRIu64 " found in list.\n", value);
            }
        } else { // 20% writes
            uint64_t value = rand() % 100;
            if (rand() % 2) {
                add_node(list, value);
                printf("Added value %" PRIu64 " to list.\n", value);
            } else {
                if (delete_node(list, value) == 0) {
                    printf("Deleted value %" PRIu64 " from list.\n", value);
                }
            }
        }
        urcu_qsbr_quiescent_state();
    }

    return NULL;
}

int main() {
    srand(time(NULL));
    LinkedList *list = init_list();
    if (!list) {
        fprintf(stderr, "Failed to initialize the linked list.\n");
        return EXIT_FAILURE;
    }

    urcu_qsbr_register_thread();

    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, thread_func, list);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    destroy_list(list);
    urcu_qsbr_unregister_thread();

    return 0;
}
