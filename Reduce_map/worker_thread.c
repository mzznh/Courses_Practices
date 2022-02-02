#include "worker_thread.h"

/* constuctor */
/**
 * @brief create object of worker_thread struct and return it
 * 
 * @param file_name - name of the file 
 * @param start_byte - read chunk start byte in the file
 * @param end_byte - read chunk end byte in the file
 * @return worker_thread* 
 */
worker_thread *new_worker_thread(char *file_name,
                        size_t start_byte, size_t end_byte)
{
    if(file_name == NULL)
    {
        return NULL;
    }
    worker_thread *worker = NULL;
    worker = (worker_thread*) malloc(sizeof(worker_thread));
    if(worker == NULL)
    {
        return NULL;
    }
    /* init object attributes */
    worker->start_byte = start_byte;
    worker->end_byte = end_byte;
    worker->file_ptr = NULL;
    worker->thread = (pthread_t*) malloc(sizeof(pthread_t));
    if(worker->thread == NULL)
    {
        free(worker);
        return NULL;
    }
    worker->file_ptr = fopen(file_name, "r");

    if(worker->file_ptr == NULL)
    {
        free(worker);
        return NULL;
    }

    return worker;

}
/* destructor */
/**
 * @brief destroy worker_thread object and
 * 
 * @param worker - worker_thread object pointer 
 */
void destory_worker_thread(worker_thread *worker)
{
    #ifdef DEBUG
    printf("Worker with id = %lu have been destroyed\n", *worker->thread);
    #endif
    fclose(worker->file_ptr);
    free(worker->thread);
    free(worker);
}

/* operations */
/**
 * @brief this is thread callback function
 *        count number of word in byte chunk in file given by worker_thread
 *        and return the number of words counted.
 * 
 * @param self - worker_thread stuct 
 * @return void* - pointer with value of number of words counted if success
 * @return void* - NULL if error
 */
void *work (void *self)
{
    worker_thread *self_p = (worker_thread*) self;
    size_t number_of_words = 0;
    size_t byte_counter = 0;
    size_t read_chunk_size = 0;
    size_t *ret_p = NULL;
    /* set file pointer to start byte index */
    fseek(self_p->file_ptr, self_p->start_byte, SEEK_SET);
    char ch;
    char word_started = FALSE;

    ret_p = (size_t*) malloc(sizeof(size_t));
    if(ret_p == NULL)
    {
        return NULL;
    }

    /* compute read byte size */
    read_chunk_size = self_p->end_byte - self_p->start_byte;

    /* count number of word in file */
    while( (ch = fgetc(self_p->file_ptr)) != EOF && byte_counter < read_chunk_size)
    {
        #ifdef DEBUG
        printf("Debug: Thread %lu is reading %c\n", pthread_self(), ch);
        #endif
        if(ch == ' ' || ch == ',' || ch == '\n')
        {
            if(word_started)
            {
                word_started = FALSE;
                number_of_words++; 
                #ifdef DEBUG
                printf("Debug: Thread %lu number_of_word = %lu\n", pthread_self(), number_of_words);
                #endif
            }
        }
        else
        {
            if(!word_started)
            {
                word_started = TRUE;
            }
        }
        byte_counter++;

    }

    if(word_started)
    {
        /**
         * @brief check if next character is word termantion or not
         *        if word termanation count it as word 
         *        otherwise other thread will count it 
         * 
         */
        ch = fgetc(self_p->file_ptr);
        if(ch == ' ' || ch == ',' || ch == '\n' || ch == EOF)
        {
            number_of_words++;
        }
    }
    *ret_p = number_of_words;

    #ifdef DEBUG
    printf("DEBUG: Thread %lu num_of_words = %lu with address %p\n", *(self_p->thread), *ret_p, ret_p);
    #endif


    return (void*) ret_p;

}