/**
 * @file routing_table.c
 * @author ahmed gublan
 * @brief this file realted to implementing the "routing table" example as data source.
 *        this routing table will act as the data source of the publisher in Notification chain archtecture.
 * @version 0.1
 * @date 2021-11-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include"routing_table.h"
#include<stdlib.h>
#include<assert.h>
#include<string.h>
/**
 * @brief initiate routing table data source 
 * 
 * @param rt_table - valid routing table pointer
 */
void rt_init_rt_table(rt_table_t *rt_table)
{
    rt_table->head = NULL;
}
/**
 * @brief fetch routing data entry
 * 
 * @param rt_table pointer to routing table data source
 * @param dest_ip - destination ip address(part of data entry key)
 * @param mask - destination ip address mask (part of data entry key)
 * @return rt_entry_t* - pointer to fetched data entry 
 */
rt_entry_t *rt_look_up_rt_entry(rt_table_t *rt_table, 
					char *dest, char mask)
{
    rt_entry_t *fetched_entry = NULL;

    /* iterate over all linked list with <fetched_entry> pointer */
    ITERTAE_RT_TABLE_BEGIN(rt_table, fetched_entry)
    {
        /* data entry found */
        if(fetched_entry->rt_entry_key.mask == mask 
            && strncmp(fetched_entry->rt_entry_key.dest_ip, dest,
                sizeof(fetched_entry->rt_entry_key.dest_ip)) == 0 )
        {
            return fetched_entry;
        }

    }ITERTAE_RT_TABLE_END(rt_table, fetched_entry);

    /* data entry not found */
    return NULL;
}
/**
 * @brief add or update routing table data source
 *        if dest_ip and mask (entry key) exist in the table update it
 *        other wise add it as new entry 
 * 
 * @param rt_table - routing table pointer 
 * @param dest_ip - destination ip address(part of data entry key)
 * @param mask - destination ip address mask (part of data entry key)
 * @param gw_ip - gateway ip address 
 * @param oif - out interface name 
 * @return rt_entry_t* pointer added/updated entry 
 */
rt_entry_t *rt_add_or_update_rt_entry(rt_table_t *rt_table,
    char *dest_ip, char mask, char *gw_ip, char *oif)
{
    bool new_entry = false;
    rt_entry_t *rt_entry = NULL;
    rt_entry_t *head = NULL;
    
    /* check if entry exist */
    rt_entry = rt_look_up_rt_entry(rt_table, dest_ip, mask);
    
    if(!rt_entry) // entry not found, create new entry 
    {
        /* allocate data entry */
        rt_entry = (rt_entry_t*) calloc(1, sizeof(rt_entry_t));

        /* copy entry data key */
        rt_entry->rt_entry_key.mask = mask;
        if(dest_ip)
        {
            strncpy(rt_entry->rt_entry_key.dest_ip, dest_ip, MAX_IP_SIZE);
        }

        /*  subscriber notification chain data structure init */
        rt_entry->subs_notif_chain = nfc_create_new_notif_chain(NULL);

        new_entry = true;
    }

    /* copy rest of entry data */
    if(oif)
    {
        strncpy(rt_entry->out_interface_name, oif, MAX_INTERFACE_NAME);
    }
    if(gw_ip)
    {
        strncpy(rt_entry->gateway_ip, gw_ip, MAX_IP_SIZE);
    }

    if(new_entry) // add entry as a head (in start of linked list)
    {
        /* hold head data */ 
        head = rt_table->head;
        /* new entry = head -> new head */
        rt_table->head = rt_entry;
        /* connect new head with old head */
        rt_entry->prev = NULL;
        rt_entry->next = head;

        /* if there is a data in the linked list connect it to the new head */
        if(head != NULL)
        {
            head->prev = rt_entry;
        }
        
    }

    /* case publisher thread modify data entry */
    if (gw_ip || oif) {

		/* Entry is being updated by the publisher, send
 		 * notification to all subscribers*/
		nfc_invoke_notif_chain(rt_entry->subs_notif_chain,
				(char *)rt_entry, sizeof(rt_entry_t),
				NULL, 0, NFC_MOD);
	}
    return rt_entry;
}   
/**
 * @brief delete routing table data entry 
 * 
 * @param rt_table pointer to routing table data source
 * @param dest_ip - destination ip address(part of data entry key)
 * @param mask - destination ip address mask (part of data entry key)
 * 
 * @return true if Success
 *         false if Fail
 */
bool rt_delete_rt_entry(rt_table_t *rt_table,
    char *dest_ip, char mask)
{
    rt_entry_t *fetched_entry = rt_look_up_rt_entry(rt_table, dest_ip, mask);

    /* data entry found */
    if(fetched_entry)
    {
        /* remove data entry from routing table linked list */
        rt_entry_remove(rt_table, fetched_entry);

        /* destroy subscribers nfc and notfy subscribers */
        nfc_invoke_notif_chain(fetched_entry->subs_notif_chain,
            (char*)fetched_entry,
            sizeof(rt_entry_t),
            NULL, 0, NFC_DEL);
        nfc_delete_all_nfce(fetched_entry->subs_notif_chain);
        free(fetched_entry->subs_notif_chain);
        fetched_entry->subs_notif_chain = NULL;

        free(fetched_entry);
        return true;
    }

    /* data entry not found */
    return false;
}
/**
 * @brief display all data entries in routing table  
 * 
 * @param rt_table - pointer to routing table data source
 */
void rt_dump_rt_table(rt_table_t *rt_table)
{
    rt_entry_t *rt_entry_curr = NULL;

    ITERTAE_RT_TABLE_BEGIN(rt_table, rt_entry_curr)
    {
        printf("%-20s %-4d %-20s %s\n",
        rt_entry_curr->rt_entry_key.dest_ip, 
        rt_entry_curr->rt_entry_key.mask, 
        rt_entry_curr->gateway_ip,
        rt_entry_curr->out_interface_name);
        
        printf("\tPrinting Subscribers : ");
		
		glthread_t *curr;
		notif_chain_element_t *nfce;

		ITERATE_GLTHREAD_BEGIN(&rt_entry_curr->subs_notif_chain->notification_chain_head, curr)
        {

			nfce = glthread_glue_to_notif_chain_element(curr);
			
			printf("%u ", nfce->sub_id);

		} ITERATE_GLTHREAD_END(&rt_entry->nfc->notif_chain_head, curr)
		printf("\n");


    }ITERTAE_RT_TABLE_END(rt_table, rt_entry_curr);
}
/**
 * @brief this function will register a subscriber thread to a data entry
 *        Subscriber thread will call this function.
 * 
 * @param rt_table - routing table data source
 * @param key - routing table data entry key
 * @param key_size - data entry key size in bytes
 * @param app_cb - subscriber computation function pointer
 *                 this function called when we want subscriber get notified
 * @param subs_id - subscriber id
 */
void rt_table_register_for_notification(rt_table_t *rt_table,
    rt_entry_key_t *key,
    size_t key_size,
    nfc_app_cb app_cb,
    uint32_t subs_id)
{
    rt_entry_t *fetched_entry;
    notif_chain_element_t nfc_element;
    bool new_entry_created = false;
    
    /* look for subsscriber interested data entry */
    fetched_entry = rt_look_up_rt_entry(rt_table, key->dest_ip, key->mask);

    /* data entry not found */
    if(!fetched_entry)
    {
        /* rt_entry was not existing before, but we are
 		 * creating it because subscriber is interested in notif
 		 * for this entry. Create such an entry without data. Later
 		 * When publisher would actually cate this entry, all registered
 		 * subscriber should be notified
 		 * */
        fetched_entry = rt_add_or_update_rt_entry(rt_table, key->dest_ip, key->mask, NULL, NULL);
        new_entry_created = true;
    }

    /* make the notfication element for the subscriber */
    memset(&nfc_element, 0, sizeof(nfc_element));
    assert(key_size <= MAX_NOTIFI_KEY_SIZE); // validate key size

    /* fill the subscriber properties */

        /* No need to keep keys as nfce;s are tied to
        * routing table entry which contain keys*/	
        // memcpy(nfce.key, (char *)key, key_size);
        // nfce.key_size = key_size;
    nfc_element.app_cb = app_cb;
    nfc_element.sub_id = subs_id;

    /* notification chain registration */
    nfc_register_notif_chain(fetched_entry->subs_notif_chain, &nfc_element);

    /* Subscriber subscribes to already existing rt entry,
 	 * immediately send notif to Subscriber with opcode
 	 * NFC_ADD*/
    if(!new_entry_created)
    {
        app_cb(fetched_entry, sizeof(rt_entry_t), NFC_ADD, subs_id);
    }
}