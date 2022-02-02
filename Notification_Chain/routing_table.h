/**
 * @file routing_table.h
 * @author ahmed gublan
 * @brief this header file realted to implementing the "routing table" example as data source.
 *        this routing table will act as the data source of the publisher in Notification chain archtecture.
 * @version 0.1
 * @date 2021-11-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include<stdbool.h>
#include<stdlib.h>
#include<stdio.h>
#include "notification_chain.h"

/* macros */
#define MAX_IP_SIZE 16
#define MAX_INTERFACE_NAME 32

/* data source key entry */
typedef struct rt_entry_key_
{
    char dest_ip[MAX_IP_SIZE];
    char mask;
}rt_entry_key_t;

/* data source */
typedef struct rt_entry_
{
    rt_entry_key_t rt_entry_key;
    char out_interface_name[MAX_INTERFACE_NAME];
    char gateway_ip[MAX_IP_SIZE];
    struct rt_entry_ *next;
    struct rt_entry_ *prev;
    notification_chain_t *subs_notif_chain; // subscribers notification chain data structure
}rt_entry_t;

typedef struct rt_table_
{
    rt_entry_t *head;
}rt_table_t;

/*************** operations ****************/

/**
 * @brief initiate routing table data source 
 * 
 * @param rt_table - valid routing table pointer
 */
void rt_init_rt_table(rt_table_t *rt_table);

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
    char *dest_ip, char mask, char *gw_ip, char *oif);
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
    char *dest_ip, char mask);

/**
 * @brief fetch routing data entry
 * 
 * @param rt_table pointer to routing table data source
 * @param dest_ip - destination ip address(part of data entry key)
 * @param mask - destination ip address mask (part of data entry key)
 * @return rt_entry_t* - pointer to fetched data entry 
 */
rt_entry_t *rt_look_up_rt_entry(rt_table_t *rt_table, 
					char *dest, char mask);

/**
 * @brief display all data entries in routing table  
 * 
 * @param rt_table - pointer to routing table data source
 */
void rt_dump_rt_table(rt_table_t *rt_table);

/**
 * @brief remove data entry linked list node
 *        don't know why it identified here as inline but the instructor did it
 * 
 * @param rt_table - pointer to routing table data source
 * @param rt_entry - pointer to routing table entry data
 */
static inline void rt_entry_remove(rt_table_t *rt_table, rt_entry_t *rt_entry)
{
    /* case rt_entry = head */
    if(!rt_entry->prev)
    {
        if(rt_entry->next)
        {
            rt_entry->next->prev = NULL;
            rt_table->head = rt_entry->next;
            rt_entry->next = NULL;
            return ;
        }
    }
    /* case rt_entry = last node*/
    if(!rt_entry->next)
    {
        if(rt_entry->prev)
        {
            rt_entry->prev->next = NULL;
            rt_entry->prev = NULL;
            return;
        }
    }

    /* case data entry in the middle of the linked list */
    rt_entry->prev->next = rt_entry->next;
    rt_entry->next->prev = rt_entry->prev;
    rt_entry->next = NULL;
    rt_entry->prev = NULL;
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
    uint32_t subs_id);

#define ITERTAE_RT_TABLE_BEGIN(rt_table_ptr, rt_entry_ptr)                \
{                                                                         \
    rt_entry_t *_next_rt_entry;                                           \
    for((rt_entry_ptr) = (rt_table_ptr)->head;                            \
            (rt_entry_ptr);                                               \
            (rt_entry_ptr) = _next_rt_entry) {                            \
        _next_rt_entry = (rt_entry_ptr)->next;

#define ITERTAE_RT_TABLE_END(rt_table_ptr, rt_entry_ptr)  }}
#endif // ROUTING_TABLE_H
