/*

Copyright (c) 2020, CIMCON Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, is allowed only with prior permission from CIMCON Automation

*/

#include "MessageBus.h"
#include "StringEx.h"
#include "StringList.h"
#include "Responder.h"
#include "Base64.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>

#include <unistd.h>
#include <pthread.h>

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
    bool loop;
}message_bus;

static pthread_mutex_t socket_lock;
static bool read_current_process_name(void* ptr);
static bool handle_protocol(void* ptr, payload* message);
static void* responder_run(void* responder_thread_params);

LIBRARY_ENTRY void library_load()
{

}

LIBRARY_EXIT void library_unload()
{

}

bool message_bus_initialize(void** pptr, messabus_bus_callback callback)
{
    struct message_bus* message_bus_ptr = (struct message_bus*)calloc(1, sizeof (struct message_bus));
    
    if (message_bus_ptr == NULL)
    {
        return false;
    }

    *pptr = message_bus_ptr;

    strcpy(message_bus_ptr->message_bus_host, "localhost");
    message_bus_ptr->message_bus_port = 49151;
    message_bus_ptr->callback_ptr = callback;
    read_current_process_name(message_bus_ptr);
    message_bus_ptr->payload_sequence = 0;
    message_bus_ptr->loop = true;
    message_bus_ptr->peer_node_list = str_list_allocate(message_bus_ptr->peer_node_list);
    message_bus_ptr->responder = responder_create_socket(&message_bus_ptr->responder, message_bus_ptr->message_bus_host, message_bus_ptr->message_bus_port);

    pthread_mutex_init(&socket_lock, NULL);

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

    pthread_mutex_lock(&socket_lock);
    message_bus_ptr->loop = false;
    pthread_mutex_unlock(&socket_lock);

    return true;
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

    reg_payload.payload_type = Event;
    reg_payload.payload_sub_type = Register;
    strcpy(reg_payload.sender, message_bus_ptr->process_name);
    strcpy(reg_payload.receipient, "MessageBus");
    reg_payload.data_size = 0;
    reg_payload.payload_id = message_bus_ptr->payload_sequence;

    return responder_send_buffer(message_bus_ptr->responder, &reg_payload, sizeof (struct payload) - sizeof (char*));
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

    dereg_payload.payload_type = Event;
    dereg_payload.payload_sub_type = DeRegister;
    strcpy(dereg_payload.sender, message_bus_ptr->process_name);
    strcpy(dereg_payload.receipient, "MessageBus");
    dereg_payload.data_size = 0;
    dereg_payload.payload_id = message_bus_ptr->payload_sequence;

    return responder_send_buffer(message_bus_ptr->responder, &dereg_payload, sizeof (struct payload) - sizeof (char*));
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

    long datalen = 0;
    void* dataptr = NULL;

    if(dtype == Text)
    {
        datalen = buffersize;
        dataptr = messagebuffer;
    }
    else
    {
        dataptr = base64_encode(messagebuffer, buffersize, dataptr, &datalen);
    }

    payload data_payload = {0};

    data_payload.payload_type = ptype;
    data_payload.payload_sub_type = mtype;
    data_payload.payload_data_type = dtype;
    strcpy(data_payload.sender, message_bus_ptr->process_name);
    strcpy(data_payload.receipient, node_name);
    data_payload.data_size = datalen;
    data_payload.payload_id = message_bus_ptr->payload_sequence;

    if(!responder_send_buffer(message_bus_ptr->responder, &data_payload, sizeof (struct payload) - sizeof (char*)))
    {
        return false;
    }

    if(!responder_send_buffer(message_bus_ptr->responder, dataptr, (size_t)datalen))
    {
        return false;
    }

    if(dtype != Text)
    {
        free(dataptr);
    }

    return true;
}

bool message_bus_send_loopback(void* ptr)
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

    payload loopback_payload = {0};

    loopback_payload.payload_type = Data;
    loopback_payload.payload_sub_type = LoopBack;
    loopback_payload.payload_data_type = Text;
    strcpy(loopback_payload.sender, message_bus_ptr->process_name);
    strcpy(loopback_payload.receipient, "MessageBus");
    loopback_payload.data_size = strlen("LOOPBACK");
    loopback_payload.data = (char*)calloc(1, loopback_payload.data_size + 1);
    strcpy(loopback_payload.data, "LOOPBACK");
    loopback_payload.payload_id = message_bus_ptr->payload_sequence;

    if(!responder_send_buffer(message_bus_ptr->responder, &loopback_payload, sizeof (struct payload) - sizeof (char*)))
    {
        return false;
    }

    if(!responder_send_buffer(message_bus_ptr->responder, loopback_payload.data, loopback_payload.data_size))
    {
        return false;
    }

    free(loopback_payload.data);

    return true;
}

char* message_bus_localname(void* ptr)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return NULL;
    }

    return message_bus_ptr->process_name;
}

long message_bus_has_node(void* ptr, const char* node_name)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&socket_lock);

    if (message_bus_ptr == NULL)
    {
        return -1;
    }

    if (message_bus_ptr->peer_node_list == NULL)
    {
        return -1;
    }

    long index = str_list_index_of_like(message_bus_ptr->peer_node_list, node_name);

    pthread_mutex_unlock(&socket_lock);

    return index;
}

char* message_bus_node_fullname(void* ptr, long node_index)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    char* str = NULL;

    if (message_bus_ptr == NULL || node_index < 0)
    {
        return NULL;
    }

    pthread_mutex_lock(&socket_lock);

    if ((str_list_item_count(message_bus_ptr->peer_node_list)-1) < node_index)
    {
        str = NULL;
    }
    else
    {
        str = str_list_get_at(message_bus_ptr->peer_node_list, node_index);
    }

    pthread_mutex_unlock(&socket_lock);

    return str;
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

        if(responder_receive_buffer(message_bus_ptr->responder,  &buffer, sizeof (struct payload) - sizeof (char*), false) && message_bus_ptr->loop == true)
        {
            if(message.data_size > 0)
            {
                char* databuffer = NULL;
                if(!responder_receive_buffer(message_bus_ptr->responder, &databuffer, message.data_size, true))
                {
                    break;
                }

                databuffer[message.data_size] = 0;

                if(message.payload_data_type == Text)
                {
                    message.data = databuffer;
                }
                else
                {
                    long decoded_len = 0;
                    message.data = base64_decode(databuffer, message.data_size, message.data, &decoded_len);
                    message.data_size = decoded_len;
                    free(databuffer);
                }
            }

            handle_protocol(message_bus_ptr, &message);

            if (message.data_size > 0 && message.data != NULL)
            {
                free(message.data);
            }
        }
        else
        {
            break;
        }
    }

    responder_close_socket(message_bus_ptr->responder);
    str_list_clear(message_bus_ptr->peer_node_list);
    free(message_bus_ptr);
    return NULL;
}

bool handle_protocol(void* ptr, payload* message)
{
    message_bus* message_bus_ptr = (struct message_bus*)ptr;

    if(message_bus_ptr == NULL)
    {
        return false;
    }

    // We get this once we connect and register ourselves
    if(message->payload_sub_type == NodeList)
    {
        pthread_mutex_lock(&socket_lock);

        void* temp_list = NULL;

        temp_list = str_list_allocate_from_string(temp_list, message->data, ",");

        char* str = str_list_get_first(temp_list);

        while (str)
        {
            str_list_add_to_tail(message_bus_ptr->peer_node_list, str);

            str = str_list_get_next(temp_list);
        }

        if (temp_list)
        {
            str_list_clear(temp_list);
        }

        pthread_mutex_unlock(&socket_lock);

        return true;
    }

    // We get this when there is a REGISTER at the server except ours own
    if(message->payload_sub_type == NodeOnline)
    {
        pthread_mutex_lock(&socket_lock);

        long long index = str_list_index_of(message_bus_ptr->peer_node_list, message->data);

        if(index < 0)
        {
            str_list_add_to_tail(message_bus_ptr->peer_node_list, message->data);
        }

        pthread_mutex_unlock(&socket_lock);
    }

    // We get this when there is a DEREGISTER at the server
    if(message->payload_sub_type == NodeOffline)
    {
        pthread_mutex_lock(&socket_lock);

        long long index = str_list_index_of_like(message_bus_ptr->peer_node_list, message->data);

        if(index > -1)
        {
            str_list_remove_at(message_bus_ptr->peer_node_list, index);
        }

        pthread_mutex_unlock(&socket_lock);
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
    pid_t proc_id = getpid();

    sprintf(buffer, "/proc/%d/cmdline", proc_id);

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
            else
            {
                void* dir_tokens = NULL;
                dir_tokens = str_list_allocate_from_string(dir_tokens, buffer, "/");

                if(dir_tokens && str_list_item_count(dir_tokens) > 0)
                {
                    strcpy(message_bus_ptr->process_name, str_list_get_last(dir_tokens));
                    free(dir_tokens);
                }
            }
        }

        fclose(fp);
    }
}
