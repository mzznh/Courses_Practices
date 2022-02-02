#include "traffic_light.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>

/**************************************** Traffic Light functions BEGIN ****************************************/

void traffic_light_init(traffic_light_t *traffic_light)
{
    for(int i = 0; i<MAX_DIRECTION; i++)
    {
        traffic_light->traffic_light_faces[i].color = RED;
        pthread_mutex_init(&traffic_light->traffic_light_faces[i].mutex, NULL);
        wait_queue_init(&traffic_light->traffic_light_faces[i].wq);
    }
}
void traffic_light_set_status(traffic_light_t *traffic_light, direction_t dir,
        traffic_light_color color)
{
    traffic_light->traffic_light_faces[dir].color = color;
    
    /* if light is green - let all blocked traffic flow */
    if(color == GREEN)
    {
        /* signal traffics waiting in wait_queue with no locking - locking handled already */
        wait_queue_broadcast(&traffic_light->traffic_light_faces[dir].wq, false);
    }
}

/**************************************** Traffic Light functions END ****************************************/


/**************************************** Utils functions BEGIN ****************************************/
static void file_write(char *buff) {

    static FILE *fptr = NULL;

    if (!fptr) {
        fptr = fopen ("log", "w");
    }

    fwrite(buff, sizeof(char), strlen(buff), fptr);
    fflush(fptr);
}
/**************************************** Utils functions END ****************************************/



/**************************************** Traffic functions BEGIN ****************************************/

/**
 * @brief   function to test the condition whenever traffic should flow or not
 *          compatible with wait_queue_t test condition function signature 
 * 
 * @param arg       traffic_t - traffic object pointer 
 * @param mutex     traffic light mutex 
 * 
 * @return  true     stop traffic  
 *          false    flow traffic 
 */
static bool traffic_check_stop_condition(void *arg, pthread_mutex_t **mutex)
{
    traffic_t *traffic = (traffic_t*) arg;

    if(mutex != NULL)
    {
        *mutex = &traffic->traffic_light->traffic_light_faces[traffic->traffic_direction].mutex;
        pthread_mutex_lock(*mutex);
    }

    /* check traffic facing light color */
    if(traffic->traffic_light->traffic_light_faces[traffic->traffic_direction].color == RED)
    {
        return true;
    }
    return false;
}
/**
 * @brief   traffic function callback
 *          this function is run the traffic routine
 * 
 * @param arg       traffic_t - traffic object 
 * @return void* 
 */
static void *traffic_function_cb(void *arg)
{
    traffic_t *traffic = (traffic_t *) arg;
    traffic_light_t *traffic_light = traffic->traffic_light;
    static char log_buff[256];

    while(1)
    {
        /* test and stop if traffic light is red */
        wait_queue_test_and_wait(&traffic_light->traffic_light_faces[traffic->traffic_direction].wq, 
            traffic->stop_traffic, arg);


        /**
         * traffic not stopping in traffic light 
         * simulate traffic flowing by writing to file
         */
        if(traffic->traffic_light->traffic_light_faces[traffic->traffic_direction].color == GREEN)
        {
            sprintf(log_buff, "Traffic %s is flowing \n",traffic->thread->name);
        }

        if(traffic->traffic_light->traffic_light_faces[traffic->traffic_direction].color == YELLOW)
        {
            sprintf(log_buff, "Traffic %s is slowing \n",traffic->thread->name);
        }
        
        file_write(log_buff);
        /* exit critical section - release traffic light mutex */
        pthread_mutex_unlock(&traffic_light->traffic_light_faces[traffic->traffic_direction].mutex);
        
        sleep(2);
    }
}

void traffic_init(traffic_t *traffic, direction_t traffic_dir, traffic_light_t *traffic_light)
{
    static int east_traffic_number = 1;
    static int west_traffic_number = 1;
    static int north_traffic_number = 1;
    static int south_traffic_number = 1;
    char traffic_name[32];

    switch (traffic_dir)
    {
        case EAST:
        {
            sprintf(traffic_name, "%s%d", "TH_EAST", east_traffic_number);
            east_traffic_number++;
            break;
        }
        case WEST:
        {
            sprintf(traffic_name, "%s%d", "TH_WEST", west_traffic_number);
            west_traffic_number++;
            break;
        }
        case NORTH:
        {
            sprintf(traffic_name, "%s%d", "TH_NORTH", north_traffic_number);
            north_traffic_number++;
            break;
        }
        case SOUTH:
        {
            sprintf(traffic_name, "%s%d", "TH_SOUTH", south_traffic_number);
            south_traffic_number++;
            break;
        }
        default:
        {
            sprintf(traffic_name, "%s%d", "TH_EAST", east_traffic_number);   
            east_traffic_number++;
            break;
        }
    }

    traffic->thread = thread_create(NULL, traffic_name);
    traffic->traffic_direction = traffic_dir;
    traffic->traffic_light = traffic_light;
    traffic->traffic_status = TRAFFIC_RUN_NORMAL;
    traffic->stop_traffic = traffic_check_stop_condition;

}
void traffic_run(traffic_t *traffic, void*(*traffic_function)(void* ))
{
    thread_run(traffic->thread, traffic_function, traffic);
}


/**************************************** Traffic functions END ****************************************/


/**************************************** Application BEGIN ****************************************/

void user_menu(traffic_light_t *traffic_light) {

    int choice ;
    direction_t dirn;
    traffic_light_color col;

    while (1) {

        printf("Traffic light Operation : \n");
        printf ("1. East : Red \n");
        printf ("2. East : Yellow \n");
        printf ("3. East : Green \n");
        printf ("4. West : Red \n");
        printf ("5. West : Yellow \n");
        printf ("6. West : Green \n");
        printf ("7. North : Red \n");
        printf ("8. North : Yellow \n");
        printf ("9. North : Green \n");
        printf ("10. South : Red \n");
        printf ("11. South : Yellow \n");
        printf ("12. South : Green \n");
        printf ("Enter Choice : ");

        scanf("%d", &choice);

        switch(choice) {

                case 1:
                    dirn = EAST; col = RED;
                    break;
                case 2:
                    dirn = EAST ; col = YELLOW;
                    break;
                case 3:
                    dirn = EAST ; col = GREEN;
                    break;
                case 4:
                    dirn = WEST; col = RED;
                    break;
                case 5:
                    dirn = WEST ; col = YELLOW;
                    break;
                case 6:
                    dirn = WEST ; col = GREEN;
                    break;
                case 7:
                    dirn = NORTH; col = RED;
                    break;
                case 8:
                    dirn = NORTH ; col = YELLOW;
                    break;
                case 9:
                    dirn = NORTH ; col = GREEN;
                    break;
                case 10:
                    dirn = SOUTH; col = RED;
                    break;
                case 11 :
                    dirn = SOUTH ; col = YELLOW;
                    break;
                case 12:
                    dirn = SOUTH ; col = GREEN;
                    break;
                default : ;
        }

        /* lock mutex (shared resource) */
        pthread_mutex_lock(&traffic_light->traffic_light_faces[dirn].mutex);
        
        traffic_light_set_status(traffic_light, dirn, col);
        
        pthread_mutex_unlock(&traffic_light->traffic_light_faces[dirn].mutex);

    } // while ends
}

int main(int argc, char **argv) {

    traffic_light_t *traffic_light;

    traffic_light = calloc(1 , sizeof(traffic_light_t));

    /* launching traffics */

    /* traffic 1 */
    traffic_t *traffic1 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic1, EAST, traffic_light);
    traffic_run(traffic1, traffic_function_cb);

    /* traffic 2 */
    traffic_t *traffic2 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic2, EAST, traffic_light);
    traffic_run(traffic2, traffic_function_cb);

    /* traffic 3 */
    traffic_t *traffic3 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic3, WEST, traffic_light);
    traffic_run(traffic3, traffic_function_cb);

    /* traffic 4 */
    traffic_t *traffic4 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic4, WEST, traffic_light);
    traffic_run(traffic4, traffic_function_cb);

    /* traffic 5 */
    traffic_t *traffic5 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic5, NORTH, traffic_light);
    traffic_run(traffic5, traffic_function_cb);

    /* traffic 6 */
    traffic_t *traffic6 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic6, NORTH, traffic_light);
    traffic_run(traffic6, traffic_function_cb);

    /* traffic 7 */
    traffic_t *traffic7 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic7, SOUTH, traffic_light);
    traffic_run(traffic7, traffic_function_cb);

    /* traffic 8 */
    traffic_t *traffic8 = calloc(1, sizeof(traffic_light_t));
    traffic_init(traffic8, SOUTH, traffic_light);
    traffic_run(traffic8, traffic_function_cb);

    /* initiate traffic light */
    traffic_light_init(traffic_light);

    user_menu(traffic_light);

    pthread_exit(0);
}

/**************************************** Application END ****************************************/
