#include "MessageBroker.h"
#include "Logger.h"
#include "StringEx.h"
#include "Responder.h"
#include "Payload.h"
#include "SignalHandler.h"

#include <pthread.h>
#include <memory.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR	 (-1)
#define LPSOCKADDR sockaddr*
#define MAX_RESPONDERS 65535

#ifndef ERESTART
#define ERESTART 99
#endif

static void on_signal_received(SignalType type);
static void *responder_run(void* responder_thread_params);
static void payload_handle_protocol(payload* message, void* vptr_responder);
static bool payload_receive(payload* message, void* vptr_responder);
static bool payload_send(payload* message, void* vptr_responder);

static int listener_socket = INVALID_SOCKET;
static int listener_port = -1;
static size_t logger_id = 0;

static socklen_t addrlen;
static pthread_mutex_t socket_mutex;

typedef struct responder_thread_params
{
    pthread_t thread;
    void* responder;
}responder_thread_params;

typedef struct client_node
{
    char node_name[32];
    void* responder;
}client_node;

static void* client_node_array[MAX_RESPONDERS];

bool broker_initialize(char* appname, int port)
{
    listener_port = port;

    logger_id = logger_allocate(10, appname, NULL);
    logger_start_logging(logger_id);

    signals_register_callback(on_signal_received);
    signals_initialize_handlers();

    pthread_mutex_init(&socket_mutex, NULL);

    return true;
}

bool broker_run()
{
    listener_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    struct sockaddr_in bindAddr;

    memset((void*)&bindAddr, 0, sizeof(struct sockaddr_in));

    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(listener_port);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listener_socket, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
    {
        return BindFailed;
    }

    if(listen(listener_socket,5)==SOCKET_ERROR)
    {
        return ListenFailed;
    }

    while(true)
    {
        struct sockaddr remotehostaddr;
        memset((void*)&remotehostaddr, 0, sizeof(remotehostaddr));
        addrlen = sizeof(remotehostaddr);

        int client_sock = accept(listener_socket,&remotehostaddr,&addrlen);
        if(client_sock != INVALID_SOCKET)
        {
            struct responder_thread_params* params = (responder_thread_params*)calloc(1, sizeof (struct responder_thread_params));
            params->responder = responder_assign_socket(params->responder, client_sock);

            pthread_attr_t pthread_attr;
            memset(&pthread_attr, 0, sizeof(pthread_attr_t));
            // default threading attributes
            pthread_attr_init(&pthread_attr);
            // allow a thread to exit cleanly without a join
            pthread_attr_setdetachstate (&pthread_attr,PTHREAD_CREATE_DETACHED);
            if (pthread_create(&params->thread, &pthread_attr, responder_run, params) != 0)
            {
                responder_close_socket(params->responder);
                free(params);
                continue;
            }
        }
        else
        {
            if ((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR))
            {
                return SignalReceived;
            }
        }
    }
}

void broker_stop()
{
    shutdown(listener_socket, 2);
    close(listener_socket);
    pthread_mutex_destroy(&socket_mutex);
}

void* responder_run(void* responder_thread_params)
{
    pthread_detach(pthread_self());

    struct responder_thread_params* params = (struct responder_thread_params*)responder_thread_params;

    if(params == NULL)
    {
        pthread_exit(NULL);
        return NULL;
    }

    if(params->responder == NULL)
    {
        pthread_cancel(params->thread);
        free(responder_thread_params);
        pthread_exit(NULL);
        return NULL;
    }

    while(true)
    {
        payload client_payload = {0};

        if(payload_receive(&client_payload, params->responder))
        {
            payload_handle_protocol(&client_payload, params->responder);
            free(client_payload.data);
        }
        else
        {
            break;
        }
    }

    pthread_cancel(params->thread);

    // Lock this section for responder array update

    pthread_mutex_lock(&socket_mutex);

    int curr_socket = responder_get_socket(params->responder);
    responder_close_socket(params->responder);
    client_node_array[curr_socket] = NULL;

    pthread_mutex_unlock(&socket_mutex);

    // Responder array update complete

    free(params);

    pthread_exit(NULL);
}

void payload_handle_protocol(payload* message, void* vptr_responder)
{
    struct responder* responder_ptr = (struct responder*)vptr_responder;

    if(!responder_ptr)
    {
        return;
    }

    int sender_socket = responder_get_socket(responder_ptr);

    // Event => Registration
    if(message->payload_type == PAYLOAD_TYPE_EVENT && message->payload_sub_type == PAYLOAD_SUB_TYPE_REGISTER)
    {
        pthread_mutex_lock(&socket_mutex);

        // Make sure that the registration request is from a new node
        client_node* new_node = client_node_array[sender_socket];

        if(new_node == NULL)
        {
            new_node = (struct client_node*)calloc(1, sizeof (struct client_node));
            strcpy(new_node->node_name, message->sender);
            new_node->responder = responder_ptr;
            client_node_array[sender_socket] = new_node;
        }

        // Send Node Online event to all other nodes

        // While we iterate, we will also build up a node list payload for the new node
        struct payload node_list_message = {0};
        node_list_message.payload_type = PAYLOAD_TYPE_EVENT;
        node_list_message.payload_sub_type = PAYLOAD_SUB_TYPE_NODELIST;
        strcpy(node_list_message.receipient, message->sender);
        strcpy(node_list_message.sender, "MessageBus");

        for(size_t index = 0; index < MAX_RESPONDERS; index++)
        {
            if(client_node_array[index] == NULL)
            {
                continue;
            }

            client_node* ptr = client_node_array[index];

            if(strcmp(ptr->node_name, message->sender) == 0)
            {
                continue;
            }

            struct payload node_online_message = {0};
            node_online_message.payload_type = PAYLOAD_TYPE_EVENT;
            node_online_message.payload_sub_type = PAYLOAD_SUB_TYPE_NODE_ONLINE;
            strcpy(node_online_message.receipient, ptr->node_name);
            strcpy(node_online_message.sender, "MessageBus");
            node_online_message.data_size = strlen(message->sender);
            node_online_message.data = calloc(1, node_online_message.data_size + 1);
            strcpy((char*)node_online_message.data, message->sender);

            node_list_message.data = realloc(node_list_message.data, strlen(ptr->node_name) + 2);
            strcat((char*)node_list_message.data, ptr->node_name);
            strcat((char*)node_list_message.data, ",");

            payload_send(&node_online_message, ptr->responder);
            free(node_online_message.data);
        }

        if(node_list_message.data)
        {
            ((char*)node_list_message.data)[strlen((char*)node_list_message.data) - 1] = 0;
            node_list_message.data_size = strlen((char*)node_list_message.data);

            // Now send Node List event to the newly added node
            payload_send(&node_list_message, responder_ptr);
            free(node_list_message.data);
        }

        pthread_mutex_unlock(&socket_mutex);
        return;
    }

    // Event => Deregistration
    if(message->payload_type == PAYLOAD_TYPE_EVENT && message->payload_sub_type == PAYLOAD_SUB_TYPE_DEREGISTER)
    {
        pthread_mutex_lock(&socket_mutex);

        client_node* old_node = client_node_array[sender_socket];

        // If we have the node in our array
        if(old_node != NULL)
        {
            // Send Node Offline event to all other nodes
            for(size_t index = 0; index < MAX_RESPONDERS; index++)
            {
                if(client_node_array[index] == NULL)
                {
                    continue;
                }

                client_node* ptr = client_node_array[index];

                if(strcmp(ptr->node_name, message->sender) == 0)
                {
                    continue;
                }

                struct payload node_offline_message;
                node_offline_message.payload_type = PAYLOAD_TYPE_EVENT;
                node_offline_message.payload_sub_type = PAYLOAD_SUB_TYPE_NODE_OFFLINE;
                strcpy(node_offline_message.receipient, ptr->node_name);
                strcpy(node_offline_message.sender, "MessageBus");
                node_offline_message.data_size = strlen(message->sender);
                node_offline_message.data = calloc(1, node_offline_message.data_size + 1);
                strcpy((char*)node_offline_message.data, message->sender);

                payload_send(&node_offline_message, ptr->responder);
                free(node_offline_message.data);
            }

            // Now remove the old node from the array
            responder_close_socket(old_node->responder);
            client_node_array[sender_socket] = NULL;
        }

        pthread_mutex_unlock(&socket_mutex);
        return;
    }

    //Loopback
    if(message->payload_type == PAYLOAD_TYPE_DATA && message->payload_sub_type == PAYLOAD_SUB_TYPE_LOOPBACK)
    {
        payload_send(message, responder_ptr);
        return;
    }

    // All other payload types that carry trailing data buffers
    if(message->payload_type == PAYLOAD_TYPE_DATA || message->payload_type == PAYLOAD_TYPE_REQUEST || message->payload_type == PAYLOAD_TYPE_RESPONSE || message->payload_type ==  PAYLOAD_TYPE_EVENT)
    {
        pthread_mutex_lock(&socket_mutex);

        for(size_t index = 0; index < MAX_RESPONDERS; index++)
        {
            client_node* ptr = client_node_array[index];

            if(ptr != NULL)
            {
                if(strcmp(ptr->node_name, message->receipient) == 0)
                {
                    payload_send(message, ptr->responder);
                    break;
                }
            }
        }

        pthread_mutex_unlock(&socket_mutex);
        return;
    }
}

bool payload_receive(payload* message, void* vptr_responder)
{
    if(!message)
    {
        return false;
    }

    struct responder* responder_ptr = (struct responder*)vptr_responder;

    if(!responder_ptr)
    {
        return  false;
    }

    if(!responder_is_connected(responder_ptr))
    {
        return false;
    }

    if(!responder_receive_buffer(responder_ptr, (char*)&message, sizeof (struct payload) - sizeof(void*), false))
    {
        return false;
    }

    if(message->data_size > 0)
    {
        if(!responder_receive_buffer(responder_ptr, &message->data, message->data_size, true))
        {
            return false;
        }
    }

    return true;
}

bool payload_send(payload* message, void* vptr_responder)
{
    if(!message)
    {
        return false;
    }

    struct responder* responder_ptr = (struct responder*)vptr_responder;

    if(!responder_ptr)
    {
        return  false;
    }

    if(!responder_is_connected(responder_ptr))
    {
        return false;
    }

    if(!responder_send_buffer(responder_ptr, (const char*)message, sizeof (struct payload) - sizeof (void*)))
    {
        return false;
    }

    if(message->data_size > 0 && message->data != NULL)
    {
        if(!responder_send_buffer(responder_ptr, (const char*)message->data, message->data_size))
        {
            return false;
        }
    }

    return true;
}

void on_signal_received(SignalType stype)
{
    switch(stype)
    {
        case Suspend:
        {
            WriteLog(logger_id, "SUSPEND SIGNAL", LOG_CRITICAL);
            break;
        }
        case Resume:
        {
            WriteLog(logger_id, "RESUME SIGNAL", LOG_CRITICAL);
            break;
        }
        case Shutdown:
        {
            WriteLog(logger_id, "SHUTDOWN SIGNAL", LOG_CRITICAL);
            exit(0);
        }
        case Alarm:
        {
            WriteLog(logger_id, "ALARM SIGNAL", LOG_CRITICAL);
            break;
        }
        case Reset:
        {
            WriteLog(logger_id, "RESET SIGNAL", LOG_CRITICAL);
            break;
        }
        case ChildExit:
        {
            WriteLog(logger_id, "CHILD PROCESS EXIT SIGNAL", LOG_CRITICAL);
            break;
        }
        case Userdefined1:
        {
            WriteLog(logger_id, "USER DEFINED 1 SIGNAL", LOG_CRITICAL);
            break;
        }
        case Userdefined2:
        {
            WriteLog(logger_id, "USER DEFINED 2 SIGNAL", LOG_CRITICAL);
            break;
        }
        default:
        {
            WriteLog(logger_id, "UNKNOWN SIGNAL", LOG_CRITICAL);
            break;
        }
    }
}
