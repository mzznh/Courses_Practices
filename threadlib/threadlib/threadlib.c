/**
 * @file threadlib.c
 * @author ahmed gublan
 * @brief  This file represents the thread library created over POSIX library 
 * @version 0.1
 * @date 2022-01-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "threadlib.h"
#include "stdlib.h"
#include "memory.h"
#include "stdio.h"
#include "bitsop.h"
#include <assert.h>

thread_t *thread_create(thread_t *thread, char *name)
{
    if(thread == NULL)
    {
        thread = calloc(1, sizeof(thread_t));
    }
    strncpy(thread->name, name, sizeof(thread->name));
    thread->thread_created = false;
    thread->thread_fn = NULL;
    thread->arg = NULL;
    pthread_attr_init(&thread->attributes);
    thread->thread_pause_fn = NULL;
    thread->pause_arg = NULL;
    thread->flag = 0;
    pthread_mutex_init(&thread->state_mutex, NULL);
    pthread_cond_init(&thread->cv, NULL);

    return thread;

}
void thread_run(thread_t *thread, void *(*thread_fn)(void *), void *arg)
{
    thread->thread_fn = thread_fn;
    thread->arg = arg;
    thread->thread_created = true;
    thread->flag = THREAD_F_RUNNING;
    pthread_create(&thread->thread, &thread->attributes, thread->thread_fn, thread->arg);

}
void thread_set_thread_attribute_joinable_or_detached(thread_t *thread, bool joinable)
{
    pthread_attr_setdetachstate(&thread->attributes, joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED);
}

void thread_set_pause_fn(thread_t *thread,
                    void *(*thread_pause_fn)(void *),
                    void *pause_arg)
{
    thread->thread_pause_fn = thread_pause_fn;
    thread->pause_arg = pause_arg;
}
void thread_pause(thread_t *thread)
{
    pthread_mutex_lock(&thread->state_mutex);
    if(IS_BIT_SET(thread->flag, THREAD_F_RUNNING)) // check if thread is running at first 
    {
        SET_BIT(thread->flag, THREAD_F_MARKED_FOR_PAUSE);
    }
    pthread_mutex_unlock(&thread->state_mutex);
}
void thread_resume(thread_t *thread)
{
    pthread_mutex_lock(&thread->state_mutex);
    if(IS_BIT_SET(thread->flag, THREAD_F_PAUSED)) // check if thread pause at first 
    {
        pthread_cond_signal(&thread->cv);
    }
    pthread_mutex_unlock(&thread->state_mutex);
}
void thread_test_and_pause(thread_t *thread)
{
    /* lock */
    pthread_mutex_lock(&thread->state_mutex);
    /* test pause */
    if(IS_BIT_SET(thread->flag, THREAD_F_MARKED_FOR_PAUSE))
    {
        SET_BIT(thread->flag, THREAD_F_PAUSED); // set flag 
        UNSET_BIT(thread->flag, THREAD_F_MARKED_FOR_PAUSE); // unset flag
        UNSET_BIT(thread->flag, THREAD_F_RUNNING); // unset flag
        pthread_cond_wait(&thread->cv, &thread->state_mutex); // thread paused

        /* thread wakeup (resume) here */
        SET_BIT(thread->flag, THREAD_F_RUNNING); // set flag 
        thread->thread_pause_fn(thread->pause_arg);
    }
    pthread_mutex_unlock(&thread->state_mutex);

}

void thread_pool_init(thread_pool_t *th_pool)
{
    init_glthread(&th_pool->pool_head);
    pthread_mutex_init(&th_pool->mutex, NULL);
}
void thread_pool_insert_new_thread(thread_pool_t *th_pool, thread_t *thread)
{
    pthread_mutex_lock(&th_pool->mutex);
    /* check if thread is not in a list already */
    assert(IS_GLTHREAD_LIST_EMPTY(&thread->wait_glue));

    /* check if thread is assigned to job by checking thread function */
    assert(thread->thread_fn == NULL);

    /* add thread */
    glthread_add_last(&th_pool->pool_head, &thread->wait_glue);

    pthread_mutex_unlock(&th_pool->mutex);

}
thread_t *thread_pool_get_thread(thread_pool_t *th_pool)
{
    glthread_t *node;
    thread_t *thread;
    /* mutex for Critical Section */
    pthread_mutex_lock(&th_pool->mutex);
    
    /* get node from data structure */
    node = dequeue_glthread_first(&th_pool->pool_head);
    if(node == NULL)
    {
        pthread_mutex_unlock(&th_pool->mutex);
        return NULL;
    }
    else
    {
        /* transform glthread object to thread object */
        thread = wait_glue_to_thread(node);
        pthread_mutex_unlock(&th_pool->mutex);
        return thread;
    }

}

/*********** private helper functions BEGIN **********/

/**
 * @brief   this function return thread back to thread pool
 *          Thread call this function to return it self to thread pool
 *          Notify application if request when thread returned
 * 
 * @note    1. add thread back to thread pool
 *          2. notify application if requested
 *          3. block thread with cv 
 * 
 * @param th_pool 
 * @param thread 
 */
static void thread_pool_return_thread(thread_pool_t *th_pool, thread_t *thread)
{

    /* return thread back to the pool */
    pthread_mutex_lock(&th_pool->mutex);
    glthread_add_next(&th_pool->pool_head, &thread->wait_glue);
    
    /* check if application requested for notification from worker thread */
    if(thread->semaphore != NULL)
    {
        /* notify and unblock application */
        sem_post(thread->semaphore);
    }

    /* block thread */
    pthread_cond_wait(&thread->cv, &th_pool->mutex);
    pthread_mutex_unlock(&th_pool->mutex);
}

/**
 * @brief   This function assign and run fetched thread from thread pool,
 *          
 * @note    if thread is new and not assigned to work before
 *          assign thread with work and run it
 *          otherwise just wake it up from blocking 
 *           
 * 
 * @param thread - fetched thread pointer 
 */
static void thread_pool_run_thread(thread_t *thread)
{
    /* check if thread not in a pool */
    assert(IS_GLTHREAD_LIST_EMPTY(&thread->wait_glue));

    if(!thread->thread_created)
    {
        /* create thread if it is not exist */
        thread_run(thread, thread->thread_fn, thread->arg);
    }
    else
    {
        /* unblock thread */
        pthread_cond_signal(&thread->cv);
    }

}

/**
 * @brief   thread function callback
 *          this function will be executed by thread to 
 *          execute application work and then return back to
 *          thread pool after finishing the work.
 *          All thread in thread pool must call this function when assigned to work.
 * 
 * @param arg - thread_execution_data_t - pointer to thread execution controller 
 */
static void *thread_fn_work_and_return_to_thread_pool(void *arg)
{
    thread_execution_data_t *thread_execution_data = (thread_execution_data_t *) arg;

    /* thread super loop routine */
    while(1)
    {
        /* execute work assigned to thread - stage 2 */
        thread_execution_data->thread_work_fn(thread_execution_data->arg);

        /* return back to thread pool and block it self - stage 3 */
        thread_execution_data->thread_retrun_to_thread_pool_fn(thread_execution_data->th_pool, thread_execution_data->thread);
    }
}

/*********** private helper functions END ***********/


void thread_pool_dispatch_thread(thread_pool_t *th_pool, void *(*thread_fn)(void*), void *arg, bool block_caller)
{
    thread_t *thread = NULL;
    /* fetch thread from thread pool - stage 1*/
    thread = thread_pool_get_thread(th_pool);

    /* no threads available in thread pool */
    if(thread == NULL)
    {
        return;
    }
    
    /* check if application block it self - use zero semaphore */
    if(block_caller)
    {   
        /* application want to block it self - allocate and initiate semaphore */
        thread->semaphore =  calloc(1, sizeof(sem_t));
        sem_init(thread->semaphore, 0, 0); // zero semaphore 
    }
    /* data struct to control thread execution flow - will act as argument to thread work function */
    thread_execution_data_t *thread_execution_data = (thread_execution_data_t *) thread->arg;
    
    /* check if thread execution controller data structure have been malloced before */
    if(thread_execution_data == NULL)
    {
        thread_execution_data = malloc(sizeof(thread_execution_data_t) * 1);
    }

    /* fill execution flow controller data structure */
    thread_execution_data->arg = arg;                   // thread work args 
    thread_execution_data->thread_work_fn = thread_fn;  // thread work function 
    // thread work finished function 
    thread_execution_data->thread_retrun_to_thread_pool_fn = thread_pool_return_thread;
    thread_execution_data->th_pool = th_pool;
    thread_execution_data->thread = thread;

    /* assign work to thread - stage 1 */
    thread->arg = thread_execution_data;
    /**
     * we need a function callback that all threads in thread pool going execute
     * this function will run thread on his work, then return thread back to the pool
     * this function will perform stage 2 and stage 3, in thread pool algorthim
     * This function declared as static called `thread_fn_work_and_return_to_thread_pool`
     */
    thread->thread_fn = thread_fn_work_and_return_to_thread_pool; 

    /* trigger and run thread - stage 2 and stage 3 */
    thread_pool_run_thread(thread);

    if(block_caller)
    {
        /* application block and wait for thread to finish work */
        sem_wait(thread->semaphore);

        /* worker thread notify blocked application - destroy and free semaphore */
        sem_destroy(thread->semaphore);
        free(thread->semaphore);
        thread->semaphore = NULL;
    }
}

void thread_barrier_init(th_barrier_t *barrier, 
                      uint32_t threshold_count)
{
    barrier->threshold_count = threshold_count;
    barrier->curr_wait_count = 0;
    barrier->is_ready_again = true;
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->cv, NULL);
    pthread_cond_init(&barrier->busy_cv, NULL);
}
void thread_barrier_destroy(th_barrier_t *barrier)
{
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cv);
    pthread_cond_destroy(&barrier->busy_cv);
}
void thread_barrier_wait(th_barrier_t *barrier)
{
    /* critical section */
    pthread_mutex_lock(&barrier->mutex);
    /**
     *  check if barrier disposition in progress, means threads in progress of passing barrier
     *  block thread if thraed barrier is busy  */
    while(barrier->is_ready_again == false)
    {
        pthread_cond_wait(&barrier->busy_cv, &barrier->mutex);
    }

    /* check if thread is last thread, nth thread = threshold */
    if(barrier->curr_wait_count + 1 == barrier->threshold_count)
    {
        /* disposition begin */
        barrier->is_ready_again = false;
        /* generate a relay signal (signal chain)*/
        pthread_cond_signal(&barrier->cv);
        pthread_mutex_unlock(&barrier->mutex);
        return;
    }

    /* case thread is not last thread, block thread */
    barrier->curr_wait_count++;
    pthread_cond_wait(&barrier->cv, &barrier->mutex);

    /* thraed got signaled and resumed */
    barrier->curr_wait_count--;

    /* if this thread last thread waiting on the barrier, signal threads blocked in disposition phase */
    if(barrier-> curr_wait_count == 0)
    {
        /* disposition end */
        barrier->is_ready_again = true; 
        pthread_cond_broadcast(&barrier->busy_cv);
    }
    else /* not last thread in the barrier, signal another thread block in the barrier */ 
    {
        /* signal chain */ 
        pthread_cond_signal(&barrier->cv);
    }
    pthread_mutex_unlock(&barrier->mutex);
}
void thread_barrier_signal_all(th_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex);

    /* check if there are waiting threads */
    if(barrier->curr_wait_count > 0)
    {
        pthread_cond_signal(&barrier->cv);
    }

    pthread_mutex_unlock(&barrier->mutex);
}

void wait_queue_init (wait_queue_t * wq)
{
    wq->thread_wait_count = 0;
    wq->app_mutex = NULL;
    pthread_cond_init(&wq->cv, NULL);
}

thread_t *wait_queue_test_and_wait (wait_queue_t *wq,
        wait_queue_condn_fn wait_queue_block_fn_cb,
        void *arg )
{
    bool should_block;
    pthread_mutex_t *locked_app_mutex = NULL;

    /** 
     * check application condition function in locking mode (passing mutex pointer address) 
     * store application mutex
     */
    should_block = wait_queue_block_fn_cb(arg, &locked_app_mutex);
    wq->app_mutex = locked_app_mutex;
    
    /* block thread if condition function return true */
    while(should_block)
    {
        wq->thread_wait_count++;
        pthread_cond_wait(&wq->cv, wq->app_mutex);

        /**
         * thread unlocking and resume
         * check condition function in unlocking mode 
         */
        wq->thread_wait_count--;
        should_block = wait_queue_block_fn_cb(arg, NULL);
    }
    return NULL;
}
void wait_queue_signal (wait_queue_t *wq, bool lock_mutex)
{
    /* check application mutex */
    if(!wq->app_mutex)
    {
        return;
    }
    
    /* lock mutex if application request locking */
    if(lock_mutex)
    {
        pthread_mutex_lock(wq->app_mutex);
    }

    /* check if there are threads waiting in queue */
    if(wq->thread_wait_count == 0)
    {
        if(lock_mutex)
        {
            pthread_mutex_unlock(wq->app_mutex);
            
        }
        return;
    }

    pthread_cond_signal(&wq->cv);

    if(lock_mutex)
    {
        pthread_mutex_unlock(wq->app_mutex);
    }
}
void wait_queue_broadcast (wait_queue_t *wq, bool lock_mutex)
{
        /* check application mutex */
    if(!wq->app_mutex)
    {
        return;
    }
    
    /* lock mutex if application request locking */
    if(lock_mutex)
    {
        pthread_mutex_lock(wq->app_mutex);
    }

    /* check if there are threads waiting in queue */
    if(wq->thread_wait_count == 0)
    {
        if(lock_mutex)
        {
            pthread_mutex_unlock(wq->app_mutex);
            
        }
        return;
    }

    pthread_cond_broadcast(&wq->cv);

    if(lock_mutex)
    {
        pthread_mutex_unlock(wq->app_mutex);
    }
}
void wait_queue_destroy (wait_queue_t *wq)
{
    pthread_cond_destroy(&wq->cv);
    wq->app_mutex = NULL;
}
