#ifndef WORKER_THREAD_H /* Gaurd */
#define WORKER_THREAD_H

//#define DEBUG

#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

#define FALSE 0
#define TRUE 1

/* class */
typedef struct 
{
    /* attributes */
    size_t start_byte;
    size_t end_byte;
    FILE *file_ptr;
    pthread_t *thread;

}worker_thread;

/* constuctor */
worker_thread *new_worker_thread(char *file_name,
                        size_t start_byte, size_t end_byte);

/* destructor */
void destory_worker_thread(worker_thread *worker);

/* operation */
void *work (void *self);


#endif // WORKER_THREAD_H