/**
 * @file threaded_subscriber.c
 * @author ahmed gublan
 * 
 * @brief this function implement subscriber thread in notification chain architecture.
 *        Notification chain contain a publisher own a data source.
 *        data source in this implementation is a routing table.
 *        subscriber thread will register for interested data in routing table.
 *        publisher will notify the subscriber on any update on the interested data.
 * 
 * @version 0.1
 * @date 2021-11-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "routing_table.h"
#include "notification_chain.h"
#include <pthread.h>
#include <stdint.h>
#include <memory.h>
#include <unistd.h>


/* external functions */

extern rt_table_t *publisher_get_rt_table();


/* private functions */

/**
 * @brief this function act as the computation that subscriber thread have
 *        prototype of this functoin must met the data entry notification callback
 *        this function declared as static to be used as arguemnt in other function
 * 
 * @param data_entry - routing table source data entry 
 * @param data_entry_size - size of routing table data in bytes 
 * @param nfc_op_code - notification update type  
 * @param client_id - subscriber client id
 */
static void test_cb(void *data_entry, size_t data_entry_size, nfc_op_t nfc_op_code, uint32_t client_id)
{
    rt_entry_t *rt_entry;
	printf("%s() client : %u, Notified with opcode %s\n",
			 __FUNCTION__, client_id, nfc_get_str_op_code(nfc_op_code));
	
	rt_entry = (rt_entry_t *) data_entry;
	
	printf("%-20s %-4d %-20s %s\n",
			rt_entry->rt_entry_key.dest_ip,
			rt_entry->rt_entry_key.mask,
			rt_entry->gateway_ip,
			rt_entry->out_interface_name);
	printf("\n");

}
/**
 * @brief subscriber thread callback function 
 *        subscriber will initiate routing table data entry keys
 *        Register with some routing table data source entries using the keys
 * 
 * @param client_id - uint32_t client_id  
 */
static void *subscriber_thread_fn(void *client_id)
{
    rt_entry_key_t rt_entry_key; //hold data entry key

    /* register for notification 122.1.1.1/32 */
    memset(&rt_entry_key, 0, sizeof(rt_entry_key));
    strncpy(rt_entry_key.dest_ip, "122.1.1.1", sizeof(rt_entry_key.dest_ip));
    rt_entry_key.mask = 32;
    rt_table_register_for_notification(publisher_get_rt_table(), &rt_entry_key,
        sizeof(rt_entry_key), test_cb, (uint32_t) client_id ); 

    /* register for notification 122.1.1.2/32 */
    memset(&rt_entry_key, 0, sizeof(rt_entry_key));
    strncpy(rt_entry_key.dest_ip, "122.1.1.2", sizeof(rt_entry_key.dest_ip));
    rt_entry_key.mask = 32;
    rt_table_register_for_notification(publisher_get_rt_table(), &rt_entry_key,
        sizeof(rt_entry_key), test_cb, (uint32_t) client_id ); 

    /* register for notification 122.1.1.3/32 */
    memset(&rt_entry_key, 0, sizeof(rt_entry_key));
    strncpy(rt_entry_key.dest_ip, "122.1.1.3", sizeof(rt_entry_key.dest_ip));
    rt_entry_key.mask = 32;
    rt_table_register_for_notification(publisher_get_rt_table(), &rt_entry_key,
        sizeof(rt_entry_key), test_cb, (uint32_t) client_id );

    /* pause subscriber thread so it not terminate */
    pause();

}

/* public functions */

/**
 * @brief Create a subscriber thread object
 * 
 */
void create_subscriber_thread(uint32_t client_id);


void create_subscriber_thread(uint32_t client_id)
{
    pthread_attr_t pthread_attr;
    pthread_t subs_thread;

    /* initiate pthread attribute and make detached mode */
    pthread_attr_init(&pthread_attr);
    pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);

    /* initiate thread */
    pthread_create(&subs_thread, &pthread_attr, subscriber_thread_fn, (void*) client_id);
    printf("Subscriber %u created\n", client_id);
}

