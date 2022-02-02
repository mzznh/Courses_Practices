#include"notification_chain.h"
#include<string.h>
#include<assert.h>



/**
 * @brief this function register subscription request by the subscriper 
 * 
 * @param nfc - notification chain data structure pointer 
 * @param nfce - notification chain element which has the subscription
 */
void nfc_register_notif_chain(notification_chain_t *nfc, notif_chain_element_t *nfce)
{
    /* cpy the element to local element */
    notif_chain_element_t *new_nfce = calloc(1, sizeof(notif_chain_element_t));
    memcpy(new_nfce, nfce, sizeof(notif_chain_element_t));

    /* add local element to notification data structure */
    glthread_add_last(&nfc->notification_chain_head, &new_nfce->glue);
}

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
    nfc_op_t update_type)
{
    glthread_t *curr;
    notif_chain_element_t *curr_nfce;

    /**
     * validate key size
     * asset used for cheking some exprestion
     * if expresion is true do nothing
     * else assert() will display error to stderr and abort() 
     */
    assert(key_size <= MAX_NOTIFI_KEY_SIZE);
    
    /* iterate over all glthread nodes */
    ITERATE_GLTHREAD_BEGIN(&nfc->notification_chain_head, curr)
    {
        /* get notification chain element from glthreda node */
        curr_nfce = glthread_glue_to_notif_chain_element(curr);

        /**
         * wild card case
         * 
         * if key not specified or curr_nfce->key not specified
         * means it is wild card condition 
         * so the publisher has to invoke the callback
         * 
         */
        if(!( key != NULL && key_size != 0 && curr_nfce->is_key_set && (curr_nfce->key_size == key_size) ))
        {
            curr_nfce->app_cb(arg, arg_size, update_type, curr_nfce->sub_id);
        }
        /* not wild card */
        else
        {
            /* compare the keys if match invoke the subscriber callback */
            if( memcmp(curr_nfce->key, key, key_size) == 0 )
            {
                curr_nfce->app_cb(arg, arg_size, update_type, curr_nfce->sub_id);
            }

        }
        
    }ITERATE_GLTHREAD_END(&nfc->notification_chain_head, curr); // mark for iteration end
}
/**
 * @brief allocate notification chain data structure pointer and return it
 * 
 * @param notif_chain_name - name of notification chain data structure
 * 
 * @return notification_chain_t* - pointer to allocated notifcation chain data structure 
 */
notification_chain_t *nfc_create_new_notif_chain(char *notif_chain_name)
{
    notification_chain_t *nfc = NULL;
    nfc = calloc(1, sizeof(notification_chain_t));
    if(notif_chain_name)
    {
        strncpy(nfc->nfc_name, notif_chain_name, sizeof(nfc->nfc_name));
    }
    init_glthread(&nfc->notification_chain_head);

    return nfc;
}

/**
 * @brief this function iterate over all notification chain elemenets and delete all of them.
 *  
 * 
 * @param nfc - pointer to notification chain data structure 
 */
void nfc_delete_all_nfce(notification_chain_t *nfc)
{
    glthread_t *curr;
    notif_chain_element_t *curr_nfce;
    ITERATE_GLTHREAD_BEGIN(&nfc->notification_chain_head, curr)
    {
        curr_nfce=glthread_glue_to_notif_chain_element(curr);
        remove_glthread(&curr_nfce->glue);
        free(curr_nfce);

    }ITERATE_GLTHREAD_END(nfc->notification_chain_head, curr);

}