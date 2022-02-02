/**
 * @file notification_chain.h
 * @author ahmed gublan
 * @brief notifcation chain data structure header file
 * @version 0.1
 * @date 2021-11-19
 * 
 * @copyright Copyright (c) 2021
 * compiler: gcc
 * 
 */

#ifndef NOTIFICATION_CHAIN_H
#define NOTIFICATION_CHAIN_H

#include"glthread.h"
#include"ctype.h"
#include<stdlib.h>
#include<stdbool.h>
#include<stdint.h>

/* macros defines */
#define MAX_NOTIFI_CHAIN_NAME 65
#define MAX_NOTIFI_KEY_SIZE 128





/* enums */

/**
 * @brief enum to tell the subscriber type of update 
 * 
 */

typedef enum{
    NFC_UNKNOWN,
    NFC_SUB,
    NFC_ADD,
    NFC_MOD,
    NFC_DEL,
}nfc_op_t;

/**
 * @brief this inline function will translate notification chain operations type to string
 * 
 * @param nfc_op_code - enum of the notification operation code 
 * @return char* - translated string
 */
static inline char * nfc_get_str_op_code(nfc_op_t nfc_op_code) {

	static char op_code_str[16];

	switch(nfc_op_code) {

		case NFC_UNKNOWN:
			return "NFC_UNKNOWN";
		case NFC_SUB:
			return "NFC_SUB";
		case NFC_ADD:
			return "NFC_ADD";
		case NFC_MOD:
			return "NFC_MOD";
		case NFC_DEL:
			return "NFC_DEL";
		default:
			return NULL;
	}
}

/* typedefines */

/**
 * @brief typedef for subscriber computation function pointer
 *        
 * @arg void*    - generic argument pointer
 * @arg size_t   - size of the argument in byte
 * @arg nfc_op_t - operation code (update type)
 * @arg uint32_t - client_id
 */
typedef void (*nfc_app_cb)(void *, size_t, nfc_op_t, uint32_t); 


/* NFC element struct */
typedef struct notif_chain_element_
{
    /* subscriber id */
    uint32_t sub_id;
    char key[MAX_NOTIFI_KEY_SIZE];
    size_t key_size;
    bool is_key_set;
    /* notification callback pointer*/
    nfc_app_cb app_cb;
    glthread_t glue;


}notif_chain_element_t;

/**
 * @brief get notification chain element from glthread glue
 *        calling this define in the header file will define the function to be used
 * 
 * @param glthread_t - glue pointer to glthread node
 * 
 */
GLTHREAD_TO_STRUCT(glthread_glue_to_notif_chain_element, notif_chain_element_t, glue);

/* NFC struct  */
typedef struct notification_chain_
{
    char nfc_name[MAX_NOTIFI_CHAIN_NAME];
    glthread_t notification_chain_head;
}notification_chain_t;


/*********** operations ************/

/**
 * @brief this function register subscription request by the subscriper 
 * 
 * @param nfc - notification chain data structure pointer 
 * @param nfce - notification chain element which has the subscription
 */
void nfc_register_notif_chain(notification_chain_t *nfc, notif_chain_element_t *nfce);

/**
 * @brief this function invoke the subscription request by the publisher
 * 
 * @param nfc - notification chain data structure pointer 
 * @param arg - argument to the subscriper computation
 * @param arg_size - size of arg in bytes
 * @param key - key of the notification chain element
 * @param key_size - size of key in bytes
 * @param update_type - enum for update type for subscriber notification 
 */
void nfc_invoke_notif_chain(notification_chain_t *nfc,
    void* arg, size_t arg_size,
    char *key, size_t key_size,
    nfc_op_t update_type);

/**
 * @brief allocate notification chain data structure pointer and return it
 * 
 * @param notif_chain_name - name of notification chain data structure
 * 
 * @return notification_chain_t* - pointer to allocated notifcation chain data structure 
 */
notification_chain_t *nfc_create_new_notif_chain(char *notif_chain_name);

/**
 * @brief this function iterate over all notification chain elemenets and delete all of them.
 *  
 * 
 * @param nfc - pointer to notification chain data structure 
 */
void nfc_delete_all_nfce(notification_chain_t *nfc);

#endif //NOTIFICATION_CHAIN_H

