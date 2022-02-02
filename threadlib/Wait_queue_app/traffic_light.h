#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "stdbool.h"
#include "glthread.h"
#include "threadlib.h"


/**************************************** Traffic Light BEGIN ****************************************/

/**
 * @brief   traffic light object
 *          traffic light will have 4 face in each direction
 *          each direction will have:
 *              - 3 colors light types
 *              - mutex for critical section
 *              - wait queue object in each direction to stop or move traffic in that direction.
 *          
 */

typedef enum
{
    RED,
    YELLOW,
    GREEN,
    MAX_TRAFFIC_LIGHT_COLOR,
}traffic_light_color;

typedef enum
{
    EAST,
    WEST,
    NORTH,
    SOUTH,
    MAX_DIRECTION,
}direction_t;

typedef struct traffic_light_face_
{
    traffic_light_color color;
    pthread_mutex_t mutex;
    wait_queue_t wq;

}traffic_light_face_t;

typedef struct traffic_light_
{
    traffic_light_face_t traffic_light_faces[MAX_DIRECTION];
}traffic_light_t;

/**
 * @brief initialize traffic light object 
 * 
 * @param traffic_light 
 */
void traffic_light_init(traffic_light_t *traffic_light);

/**
 * @brief set traffic light face color
 * 
 * @param traffic_light 
 * @param dir 
 * @param color 
 */
void traffic_light_set_status(traffic_light_t *traffic_light, direction_t dir,
        traffic_light_color color);


/**************************************** Traffic Light END ****************************************/

/**************************************** Traffic BEGIN ****************************************/

/* thread data structure */

/**
 * @brief   car traffics in traffic light will be simulated with threads
 *          each thread will be act as a traffic moving
 *          Threads must know:
 *              - direction which they are moving
 *              - Their own state: moving or waiting
 *              - The traffic light color in thread direction
 * 
 */


/* enum for thread status */
typedef enum
{
    TRAFFIC_RUN_SLOW,    /* traffic slow when traffic color is yellow */
    TRAFFIC_RUN_NORMAL,  /* traffic normal when traffic color is green */
    TRAFFIC_STOP,        /* traffic stop when traffic color is red */
}traffic_status;

/**
 * @brief   thread private data , moving traffic data structure
 * 
 */
typedef struct traffic_
{
    direction_t traffic_direction;
    traffic_status traffic_status;        
    thread_t *thread;                   /* thread object */
    traffic_light_t *traffic_light;     /* main application resources */
    wait_queue_condn_fn stop_traffic;   /* function that test and stop traffic */

}traffic_t;

/**
 * @brief   intiate traffic object 
 * 
 * @param traffic 
 * @param traffic_dir 
 * @param traffic_light 
 */
void traffic_init(traffic_t *traffic, direction_t traffic_dir, traffic_light_t *traffic_light);

/**
 * @brief   start run and start flow traffic 
 * 
 * @param traffic               traffic_t object 
 * @param traffic_function      traffic run function pointer
 */
void traffic_run(traffic_t *traffic, void*(*traffic_function)(void* ));

/**************************************** Traffic END ****************************************/


#endif //TRAFFIC_LIGHT_H
