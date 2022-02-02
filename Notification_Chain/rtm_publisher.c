/**
 * @file rtm_publisher.c
 * @author ahmed gublan
 * @brief this file will implement the publisher thread in Notification Chain architecture.
 *        This Notification Chain Module we will have routing table act as the data source.
 *        Routing table data source will be owned by the publisher thread
 * @version 0.1
 * @date 2021-11-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "routing_table.h"
#include <pthread.h>
#include <unistd.h>

/* macros */
#define CHOICE_ADD_UPDATE_ENTRY         1
#define CHOICE_DELETE_ENTRY             2
#define CHOICE_DUMP_TABLE               3

/* external functions */
extern void create_subscriber_thread(uint32_t client_id);


/* global variables */
rt_table_t publisher_rt_table; /* publisher routing table data source */

/* private functions */
/**
 * @brief return publisher routing table data source
 * 
 * @return rt_table_t* 
 */
rt_table_t *publisher_get_rt_table()
{
    return &publisher_rt_table;
}
/**
 * @brief prompt cli interface for user interaction
 *        cli interface to let user control the routing table data source
 * 
 */
void main_menu(void)
{
    
    int choice;
    while(1)
    {
        choice = 0;

        printf("Publisher Menu\n");
        printf("1.    Add/Update routing table entry\n");
        printf("2.    Delete routing table entry\n");
        printf("3.    Dump routing table\n");
        printf("Enter Choice :");
        scanf("%d", &choice);
        switch (choice)
        {
            case CHOICE_ADD_UPDATE_ENTRY:
            {
                char dest_ip[MAX_IP_SIZE] = "";
                char mask;
                char gate_way[MAX_IP_SIZE] = "";
                char out_interface_name[MAX_INTERFACE_NAME] = "";
                /* take data entry input */
                printf("Enter Destination ip :");
                scanf("%s", dest_ip);
                printf("dest_ip %s\n",dest_ip);
                printf("Enter Mask :");
                scanf("%hhd", &mask);
                printf("dest_ip %s\n",dest_ip);
                printf("Enter Gateway IP :");
                scanf("%s", gate_way);
                printf("Enter Interface name :");
                scanf("%s", out_interface_name);
                /* operate */
                if(rt_add_or_update_rt_entry(publisher_get_rt_table(), dest_ip,
                        mask, gate_way, out_interface_name) )
                {
                    printf("dest_ip = %s\n", dest_ip);
                    printf("Success\n");
                }
                else
                {
                    printf("Invalid Parameters !\n");
                }

            }
            break;
            case CHOICE_DELETE_ENTRY:
            {
                char dest_ip[MAX_IP_SIZE] = "";
                char mask;
                /* take data entry input */
                printf("Enter Destination ip :");
                scanf("%s", dest_ip);
                printf("Enter Mask :");
                scanf("%hhd", &mask);

                if(rt_delete_rt_entry(publisher_get_rt_table(), dest_ip, mask))
                {
                    printf("Success\n");
                }
                else
                {
                    printf("Invalid Parameters !\n");
                }
            }
            break;
            case CHOICE_DUMP_TABLE:
            {
                rt_dump_rt_table(publisher_get_rt_table());
            }
            break;
        
            default:
            {
                choice = 0;
            }
            break;
        }
    }
}
/**
 * @brief publisher thread callback function
 * 
 * @param arg - void*
 * @return void* 
 */
void *publisher_thread_fn(void *arg)
{
    /* add dummy data to routing table for demo */
    rt_add_or_update_rt_entry(publisher_get_rt_table(), "192.168.1.100", 32, "192.168.1.1", "eth01");
    rt_add_or_update_rt_entry(publisher_get_rt_table(), "192.168.1.200", 32, "192.168.1.2", "eth02");
    rt_add_or_update_rt_entry(publisher_get_rt_table(), "192.168.1.300", 32, "192.168.1.3", "eth03");

    /* display routing table */
    rt_dump_rt_table(publisher_get_rt_table());
    
    /* prompt cli interface */
    main_menu();
}
/**
 * @brief Create a publisher thread object
 * 
 */
void create_publisher_thread(void)
{
    pthread_t publisher_thread;
    pthread_attr_t pthread_attr;

    /* publisher thread in detached mode */
    pthread_attr_init(&pthread_attr);
    pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);

    /* create publisher thread */
    pthread_create(&publisher_thread, &pthread_attr, publisher_thread_fn, NULL);


}
int main(int argc, char **argv)
{
    /* initiate routing table */
    rt_init_rt_table(&publisher_rt_table);

    #if 1
    /* create 3 subscriber threads */
    create_subscriber_thread(1);
    sleep(1);
    create_subscriber_thread(2);
    sleep(1);
    create_subscriber_thread(3);
    sleep(1);
    #endif

    /* create publisher thread */
    create_publisher_thread();
    printf("Publisher thread created\n");

    /* main thread exit without waiting other threads */
    pthread_exit(EXIT_SUCCESS);
    


}
