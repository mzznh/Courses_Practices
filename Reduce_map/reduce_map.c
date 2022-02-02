/*
 * =====================================================================================
 *
 *       Filename:  reduce_map.c
 *
 *    Description: This file demonstrate the reduce map algorthim, by implementing
 *                 app that count number of words in a big file 
 *
 *        Version:  1.0
 *        Created:  2/11/2021
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *       Author: ahemd gublan
 *
 * =====================================================================================
 */

/**
 * Compile: gcc -g reduce_map.c worker_thread.c -lpthread -lm -o reduce_map
 * Run : ./reduce_map <file_name>
*/

#include "reduce_map.h"
/* global variables */
char *glob_file_name = NULL;

/* private functions prototype */
int calculate_file_size(char *file_name);
int work_splitter(size_t file_size, worker_thread ***workers_array);


int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("      Usage:  %s  <file_name>\n", argv[0]);
        printf("Description:  this application count number of words in a file\n");
        exit(EXIT_FAILURE);
    }

    size_t file_size = 0; 
    glob_file_name = argv[1];
    printf("file: %s\n", glob_file_name);
    worker_thread **workers = NULL;
    int num_of_workers = 0;
    int i = 0;
    int thread_status = 0;
    
    size_t num_of_words = 0;
    size_t *worker_return = NULL; // hold thread returns 

    /* compute file size */
    file_size = calculate_file_size(glob_file_name);
    if(file_size <= 0)
    {
        printf("Invalid file size\n");
        exit(EXIT_FAILURE);
    }
    printf("file size = %ld\n", file_size);

    /* worker thread array allocation and filling */
    num_of_workers = work_splitter(file_size, &workers);
    printf("Number of worker array = %d\n", num_of_workers);

    /* debug and check if struct array allocated and filled */
    i = 0;
    while(i < num_of_workers)
    {
        #ifdef DEBUG
        printf("DEBUG: Worker address in array = %p\n", workers[i]);
        #endif
        i++;
    }

    /* workers thread start yout job */
    i = 0;
    while(i < num_of_workers)
    {
        printf("Worker%d have been started the work\n", i);
        thread_status |= pthread_create(workers[i]->thread, NULL, work,
            (void*) workers[i]);
        i++;
    }

    if(thread_status)
    {
        printf("Error creating threads\n");
        exit(EXIT_FAILURE);
    }
    i = 0;
    while(i < num_of_workers)
    {
        pthread_join(*(workers[i]->thread), (void**) &worker_return);
        /* validate thread return */
        if(worker_return != NULL)
        {
            printf("Worker%d returns %lu \n", i, *worker_return);
            num_of_words += (*worker_return);
            /* free worker return */
            free(worker_return);
            worker_return = NULL;
        }
        else
        {
            printf("Worker%lu return NULL\n");
        }

        /* destroy worker */
        destory_worker_thread(workers[i]);

        i++;
    }

    printf("\n");
    printf("=======================================\n");
    printf("* number of the word in the file = %lu *\n", num_of_words);
    printf("=======================================\n");
    /* clean up */
    free(workers);
    return 0;
}



/**
 * @brief this function get size of file in byte
 *        
 * 
 * @param file_ptr  pointer to file that will be counted 
 * 
 * @return  file_size if success
 * @return  -1  if error
*/
int calculate_file_size(char *file_name)
{
    if(file_name == NULL)
    {
        return -1;
    }
    size_t file_size = 0;
    FILE *file_ptr = NULL;
    file_ptr = fopen(file_name, "r");
    if(file_ptr == NULL)
    {
        return -1;
    }
    fseek(file_ptr, 0L, SEEK_END);
    file_size = ftell(file_ptr);
    rewind(file_ptr);
    fclose(file_ptr);
    return file_size;
}
/**
 * @brief this function get size of file.
 *        allocate array of worker threads.
 *        num of worker threads depend on file_size and NUM_OF_THREADS.
 *        devide file byte into 3 sub sections.
 *        give each worker thread section to read.
 *         
 * 
 * @param file_size file size in bytes
 * @param workers_array address of array of workers to be allocated
 * 
 * @return  num_of_worker if success
 * @return  -1  if error
*/
int work_splitter(size_t file_size, worker_thread ***workers_array)
{
    int array_size = 0;
    int i = 0;
    size_t start_index = 0;
    size_t end_index = 0;
    double division = 0.0;
    size_t chunk_size = 0; // size of read chunk

    /* error file size */
    if(file_size <= 0L)
    {
        return -1;
    }

    if(file_size >= NUM_OF_THREADS)
    {
        array_size = NUM_OF_THREADS;
    }
    else
    {
        array_size = file_size;
    }

    /* allocate the array of workers */
    *workers_array = (worker_thread **) malloc(array_size * sizeof(worker_thread*));
    if(*workers_array == NULL)
    {
        printf("Error malloc worker_array\n");
        return -1;
    }

    while(i < array_size)
    {
        /* compute thread range of bytes to read*/
        division = (i+1) / (double) NUM_OF_THREADS;
        end_index = (size_t) round(division * file_size);
        chunk_size = end_index - start_index; // +1 to include start_index 
        
        printf("%d: range byte %lu - %lu - ", i+1, start_index, end_index);
        printf("read chunk size = %zu\n", chunk_size);

        /* make worker thread struct */
        (*workers_array)[i] = new_worker_thread(glob_file_name, start_index, end_index);
        if( (*workers_array)[i] == NULL )
        {
            free(*workers_array);
            workers_array = NULL;
            return -1;
        }
        #ifdef DEBUG
        printf("DEBUG: Worker%d have been created address = %p\n", i, (*workers_array)[i]);
        #endif


        start_index = end_index ; 
        i++;

    }

    return array_size;
}
