/**
 * @file threadlib.h
 * @author ahmed gublan
 * @brief This file defines the commonly used data structures and routines for
 *        for thread synchronization
 * @version 0.1
 * @date 2022-01-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __THREAD_LIB__
#define __THREAD_LIB__

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include "glthread.h"

/******************** thread flags status ********************/

/* thread running */
#define THREAD_F_RUNNING            (1<<0)
/* thread marked for paused, but not pause yet */
#define THREAD_F_MARKED_FOR_PAUSE   (1<<1)
/* thread blocked (paused) */
#define THREAD_F_PAUSED             (1<<2)
/* thread blocked by CV */
#define THREAD_F_BLOCKED            (1<<3)

/******************** thread flags status END ********************/

/* typedefs */
typedef struct thread_{

    char name[32];                              /* thread_t name */
    bool thread_created;                        /* flag to identify thread created or not */
    pthread_t thread;                           /* pthread object */
    void *(*thread_fn)(void *);                 /* thread work function pointer */
    void *arg;                                  /* thread work args */
    pthread_attr_t attributes;                  /* pthread object attributes */

    /* thread pause/resume */
    void *(*thread_pause_fn)(void *);           /* thread resume after pause function call */
    void *pause_arg;                            /* pause/resume function call argument */

    uint32_t flag;                              /* thread status flag */
    pthread_mutex_t state_mutex;                /* update thread state mutually exclusive */
    pthread_cond_t cv;                          /* cv on which thread will block it self */

    sem_t *semaphore;                           /* thread object semaphore - usually will be used for zero semaphore */

    glthread_t wait_glue;                       /* glthread data structure node */
} thread_t;
/**
 * @brief this macro make inline function to be used to return get the object from glue thread
 *        function name wait_glue_to_thread
 * 
 */
GLTHREAD_TO_STRUCT(wait_glue_to_thread, thread_t, wait_glue);

/**
 * @brief this function create thread object 
 * 
 * @param thread - thread pointer object
 * @param name - thread object name 
 * @return thread_t* 
 */
thread_t * thread_create(thread_t *thread, char *name);

/**
 * @brief start run thread passed in arg
 * 
 * @param thread - pointer to thread object 
 * @param thread_fn - pointer to function call 
 * @param arg - pointer to thread arguments 
 */
void thread_run(thread_t *thread, void *(*thread_fn)(void *), void *arg);

/**
 * @brief configure thread mode 
 * 
 * @param thread 
 * @param joinable 
 */
void thread_set_thread_attribute_joinable_or_detached(thread_t *thread, bool joinable);

/********************* Thead pausing and resuming *********************/

/**
 * @brief   This API set the pause function call
 *          when thread wakeup after pause, it must execute this function, before continue the work.
 * 
 * @param thread 
 * @param thread_pause_fn 
 * @param pause_arg 
 */
void thread_set_pause_fn(thread_t *thread,
                    void *(*thread_pause_fn)(void *),
                    void *pause_arg);
/**
 * @brief this API just set the pause flag
 *        thread when he got to the pause point it pause it self
 * 
 * @param thread 
 */
void thread_pause(thread_t *thread);

/**
 * @brief   this API check the pause flag
 * 
 * @param thread 
 */
void thread_resume(thread_t *thread);

/**
 * @brief   this API check the pause flag
 *          if pause flag is set pause the thread
 *          This API must be called at pause points
 * 
 * @param thread 
 */
void thread_test_and_pause(thread_t *thread);

/********************* Thead pausing and resuming END *********************/

/******************** Thread Pool Begin ********************/

/**
 * @brief thread pool data struct
 * 
 */
typedef struct thread_pool_
{
    glthread_t pool_head;
    pthread_mutex_t mutex;
}thread_pool_t;

/**
 * @brief data structure for to control threads execution flow
 *        this data structure used by thread pool function
 * 
 */
typedef struct thread_execution_data_
{
    /* attributes related to thread work */

    void *(*thread_work_fn)(void *);    /* thread work function pointer */
    void *arg;                          /* data argument to work assigned to thread */

    /* attributes related to thread after work finishing work */
    
    /* function pointer called by thread to return back to thread pool */
    void (*thread_retrun_to_thread_pool_fn)(thread_pool_t *, thread_t*); 
    thread_pool_t *th_pool;
    thread_t *thread;

}thread_execution_data_t;

/**
 * @brief initiate thread pool object
 *        caller is responsible of allocating memory for th_pool
 * 
 * @param th_pool 
 */
void thread_pool_init(thread_pool_t *th_pool);

/**
 * @brief add thread to thread pool
 *        thread require to be new and not null
 *        means should not be assigned to a job 
 * 
 * @param th_pool 
 * @param thread 
 */
void thread_pool_insert_new_thread(thread_pool_t *th_pool, thread_t *thread);

/**
 * @brief fetch a thread from thread pool
 * 
 * @param th_pool 
 * @return thread_t* 
 */
thread_t *thread_pool_get_thread(thread_pool_t *th_pool);

/**
 * @brief   This function will fetch a thread from thread pool,
 *          assign work to fetched thread, run thread on assigned work,
 *          then after finishing work thread will return it self back to the pool and block it self
 *          
 * @note    this function will implement thread pool algorthim in 3 stages
 *          stage 1: executed by application. Fetch a thread assign work to it.
 *          stage 2: thread will start to execute work assigned by application,
 *                   and finish the work.
 *          stage 3: thread will return it self back to thread pool,
 *                   and block it self.
 * 
 * @param th_pool    - pointer to thread_pool_t object
 * @param thread_fn  - pointer to thread work function
 * @param arg        - pointer to thread work arg
 */
void thread_pool_dispatch_thread(thread_pool_t *th_pool, void *(*thread_fn)(void*), void *arg, bool block_caller);

/********************* Thread pool End *********************/

/********************* Thread Barrier Begin *********************/

typedef struct th_barrier_ {

 	uint32_t threshold_count;
	uint32_t curr_wait_count;
	pthread_cond_t cv;
	pthread_mutex_t mutex;
	bool is_ready_again;
	pthread_cond_t busy_cv;
} th_barrier_t;

/**
 * @brief this function initiate thread barrier object
 * 
 * @param barrier - thread barrier object 
 * @param threshold_count - thread barrier threshold
 */
void thread_barrier_init (th_barrier_t *barrier, 
                      uint32_t threshold_count);
/**
 * @brief this function deinitialize thread barrier object
 *        freeing the object memory is user responsibility
 * 
 * @param barrier - thread barrier object 
 * @param threshold_count - thread barrier threshold
 */
void thread_barrier_destroy (th_barrier_t *barrier );

/**
 * @brief this function place a thread barrier to block threads counted below barrier threshold
 * 
 * @param barrier - thread barrier object
 */
void thread_barrier_wait (th_barrier_t *barrier);

/**
 * @brief 	force signal all threads blocked on thread barrier
 * 			this function ignores number of threads waiting on thread barrier
 * 
 * @param barrier 
 */
void thread_barrier_signal_all(th_barrier_t *barrier);


/********************* Thread Barrier End *********************/

/********************* Wait Queue Begin *********************/

typedef struct wait_queue_
{
    uint32_t thread_wait_count;     /* number of threads waiting in wait-queue */
    pthread_cond_t cv;              /* CV to block multiple threads in wait-queue */
    pthread_mutex_t *app_mutex;     /* application owned mutex cached in wait-queue */

}wait_queue_t;

/* function signature to be used by application for application condition function */
typedef bool (*wait_queue_condn_fn) (void *appln_arg, pthread_mutex_t **out_mutex);

/**
 * @brief initiate wait queue
 * 
 * @param wq 
 */
void wait_queue_init (wait_queue_t * wq);


thread_t *wait_queue_test_and_wait (wait_queue_t *wq,
        wait_queue_condn_fn wait_queue_block_fn_cb,
        void *arg );

/**
 * @brief   signal threads waiting in queue
 * 
 * @note    signal if there are threads waiting in queue
 * 
 * @param wq            wait_queue_t - pointer
 * @param lock_mutex    bool - application lock mutex indicator
 */
void wait_queue_signal (wait_queue_t *wq, bool lock_mutex);

/**
 * @brief   broadcast all threads waiting in queue
 * 
 * @note    signal if there are threads waiting in queue
 * 
 * @param wq            wait_queue_t - pointer
 * @param lock_mutex    bool - application lock mutex indicator
 */
void wait_queue_broadcast (wait_queue_t *wq, bool lock_mutex);

/**
 * @brief destory wait queue
 * 
 * @param wq 
 */
void wait_queue_destroy (wait_queue_t *wq);

/********************* Wait Queue End *********************/

#endif /* __THREAD_LIB__  */