#include <stdio.h>
#include "threadlib.h"

static th_barrier_t th_barrier;

void thread_barrier_print(th_barrier_t *th_barrier) {
    
    printf("th_barrier->threshold_count = %u\n", th_barrier->threshold_count);
    printf("th_barrier->curr_wait_count = %u\n", th_barrier->curr_wait_count);
    printf("th_barrier->is_ready_again = %s\n", th_barrier->is_ready_again ? "true" : "false");
}


void *thread_fn_callback (void *arg) {
    
    thread_barrier_wait(&th_barrier);
    
    printf("Ist barricade cleared by thread %s\n", (char *)arg); // all threads must execute this

    thread_barrier_wait(&th_barrier);
    
    printf("2nd barricade cleared by thread %s\n", (char *)arg); // all threads must execute this

    thread_barrier_wait(&th_barrier);
    
    printf("3rd barricade cleared by thread %s\n", (char *)arg); // all threads must execute this
    
    pthread_exit(0);
    return NULL;
}

static pthread_t pthreads[3];
int main()
{
    thread_barrier_init(&th_barrier, 3);
    
    static const char *th1 = "th1";
    pthread_create(&pthreads[0], NULL, thread_fn_callback, (void *)th1);
    
    static const char *th2 = "th2";
    pthread_create(&pthreads[1], NULL, thread_fn_callback, (void *)th2);
    
    static const char *th3 = "th3";
    pthread_create(&pthreads[2], NULL, thread_fn_callback, (void *)th3);
    
    pthread_join(pthreads[0], NULL);
    pthread_join(pthreads[1], NULL);
    pthread_join(pthreads[2], NULL);

    thread_barrier_print(&th_barrier);
    return 0;
}