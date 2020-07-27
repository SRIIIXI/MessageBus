/*

Copyright (c) 2020, CIMCON Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, is allowed only with prior permission from CIMCON Automation

*/

#include "MessageBusClient.h"
#include "StringEx.h"
#include "StringList.h"
#include "Responder.h"
#include "Payload.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>

#pragma push(1)
typedef struct message_bus
{
    messabus_bus_callback callback_ptr;
    void* peer_node_list;
    void* responder;
    char message_bus_host[32];
    char process_name[32];
    int message_bus_port;
    pthread_t thread;
    unsigned long payload_sequence;
}message_bus;

static bool read_current_process_name(void* ptr);
static void* responder_run(void* ptr);
static bool handle_protocol(void* ptr, payload* message);

__attribute__((constructor)) void library_load()
{

}

__attribute__((destructor)) void library_unload()
{

}

bool message_bus_initialize(void** pptr, messabus_bus_callback callback)
{
    struct message_bus* message_bus_ptr = (struct message_bus*)calloc(1, sizeof (struct message_bus));
    *pptr = message_bus_ptr;

    strcpy(message_bus_ptr->message_bus_host, "localhost");
    message_bus_ptr->message_bus_port = 49151;
    message_bus_ptr->callback_ptr = callback;
    read_current_process_name(message_bus_ptr);
    message_bus_ptr->payload_sequence = 0;

    message_bus_ptr->responder = responder_create_socket(&message_bus_ptr->responder, message_bus_ptr->message_bus_host, message_bus_ptr->message_bus_port);

    if(!message_bus_ptr->responder)
    {
        return false;
    }

    return true;
}

bool message_bus_open(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    if(!responder_connect_socket(message_bus_ptr->responder))
    {
        return false;
    }

    pthread_attr_t pthread_attr;
    memset(&pthread_attr, 0, sizeof(pthread_attr_t));
    // default threading attributes
    pthread_attr_init(&pthread_attr);
    // allow a thread to exit cleanly without a join
    pthread_attr_setdetachstate (&pthread_attr,PTHREAD_CREATE_DETACHED);
    if (pthread_create(&message_bus_ptr->thread, &pthread_attr, responder_run, message_bus_ptr) != 0)
    {
        responder_close_socket(message_bus_ptr->responder);
        return false;
    }

    return true;
}

bool message_bus_close(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    return responder_close_socket(message_bus_ptr);
}

// Node management
bool message_bus_register(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    message_bus_ptr->payload_sequence++;
    if(message_bus_ptr->payload_sequence > ULONG_MAX -1)
    {
        message_bus_ptr->payload_sequence = 1;
    }

    payload reg_payload = {0};

    reg_payload.payload_type = PAYLOAD_TYPE_EVENT;
    reg_payload.payload_sub_type = PAYLOAD_SUB_TYPE_REGISTER;
    strcpy(reg_payload.sender, message_bus_ptr->process_name);
    strcpy(reg_payload.receipient, "MessageBus");
    reg_payload.data_size = 0;
    reg_payload.payload_id = message_bus_ptr->payload_sequence;

    return responder_send_buffer(message_bus_ptr->responder, &reg_payload, sizeof (struct payload) - sizeof (void*));
}

bool message_bus_deregister(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    message_bus_ptr->payload_sequence++;
    if(message_bus_ptr->payload_sequence > ULONG_MAX -1)
    {
        message_bus_ptr->payload_sequence = 1;
    }

    payload dereg_payload = {0};

    dereg_payload.payload_type = PAYLOAD_TYPE_EVENT;
    dereg_payload.payload_sub_type = PAYLOAD_SUB_TYPE_DEREGISTER;
    strcpy(dereg_payload.sender, message_bus_ptr->process_name);
    strcpy(dereg_payload.receipient, "MessageBus");
    dereg_payload.data_size = 0;
    dereg_payload.payload_id = message_bus_ptr->payload_sequence;

    return responder_send_buffer(message_bus_ptr->responder, &dereg_payload, sizeof (struct payload) - sizeof (void*));
}

// Messaging
bool message_bus_send(void* ptr, const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long *payload_id)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    message_bus_ptr->payload_sequence++;
    if(message_bus_ptr->payload_sequence > ULONG_MAX -1)
    {
        message_bus_ptr->payload_sequence = 1;
    }

    payload data_payload = {0};

    data_payload.payload_type = ptype;
    data_payload.payload_sub_type = mtype;
    strcpy(data_payload.sender, message_bus_ptr->process_name);
    strcpy(data_payload.receipient, node_name);
    data_payload.data_size = buffersize;
    data_payload.payload_id = message_bus_ptr->payload_sequence;

    if(!responder_send_buffer(message_bus_ptr->responder, &data_payload, sizeof (struct payload) - sizeof (void*)))
    {
        return false;
    }

    if(!responder_send_buffer(message_bus_ptr->responder, messagebuffer, (size_t)buffersize))
    {
        return false;
    }

    return true;
}

bool message_bus_has_node(void* ptr, const char* node_name)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return NULL;
    }

    long index = str_list_index_of_value(message_bus_ptr->peer_node_list, node_name);

    if(index > -1)
    {
        return true;
    }

    return false;
}

void* responder_run(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    pthread_detach(pthread_self());

    if(message_bus_ptr == NULL)
    {
        free(ptr);
        return NULL;
    }

    if(!responder_is_connected(message_bus_ptr->responder))
    {
        free(ptr);
        return NULL;
    }

    while(true)
    {
        payload message = {0};
        char* buffer = (char*)&message;

        if(responder_receive_buffer(message_bus_ptr->responder,  &buffer, sizeof (struct payload) - sizeof (void*), false))
        {
            if(message.data_size > 0)
            {
                if(!responder_receive_buffer(message_bus_ptr->responder, &message.data, message.data_size, true))
                {
                    break;
                }
            }

            handle_protocol(message_bus_ptr, &message);
            free(message.data);
        }
        else
        {
            break;
        }
    }

    free(ptr);
    return NULL;
}

bool handle_protocol(void* ptr, payload* message)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    // We get this once we connect and regsiter ourselves
    if(message->payload_sub_type == PAYLOAD_SUB_TYPE_NODELIST)
    {
        str_list_allocate_from_string(message_bus_ptr->peer_node_list, message->data, ",");
        return true;
    }

    // We get this when there is a REGSITER at the server except ours own
    if(message->payload_sub_type == PAYLOAD_SUB_TYPE_NODE_ONLINE)
    {
        long long index = str_list_index_of_value(message_bus_ptr->peer_node_list, message->data);

        if(index < 0)
        {
            str_list_add_to_tail(message_bus_ptr->peer_node_list, message->data);
        }
    }

    // We get this when there is a DEREGSITER at the server
    if(message->payload_sub_type == PAYLOAD_SUB_TYPE_NODE_OFFLINE)
    {
        long long index = str_list_index_of_value(message_bus_ptr->peer_node_list, message->data);

        if(index > 1)
        {
            str_list_remove_at(message_bus_ptr->peer_node_list, index);
        }
    }

    message_bus_ptr->callback_ptr(message->sender, message->payload_type, message->payload_sub_type, message->payload_data_type, message->data, message->data_size, message->payload_id);

    return true;
}

bool read_current_process_name(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    char buffer[1025] = {0};
    pid_t pid = getpid();

    sprintf(buffer, "/proc/%d/cmdline", pid);

    FILE* fp = fopen(buffer, "r");

    if(fp)
    {
        memset(buffer, 0, 1025);

        if(fgets(buffer, 1024, fp))
        {
            void* cmd_args = NULL;
            str_list_allocate_from_string(cmd_args, buffer, " ");

            if(cmd_args && str_list_item_count(cmd_args) > 0)
            {
                void* dir_tokens = NULL;
                str_list_allocate_from_string(dir_tokens, str_list_get_first(cmd_args), "/");

                if(dir_tokens && str_list_item_count(dir_tokens) > 0)
                {
                    if(message_bus_ptr)
                    {
                        strcpy(message_bus_ptr->process_name, str_list_get_first(dir_tokens));
                    }
                    free(dir_tokens);
                }
                free(cmd_args);
            }
        }

        fclose(fp);
    }
}
