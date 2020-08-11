#include "MessageBroker.h"
#include "Logger.h"
#include "StringEx.h"
#include "Responder.h"
#include "Payload.h"
#include "SignalHandler.h"

#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#include <WinSock2.h>
#include <Windows.h>
#else
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define INVALID_SOCKET (-1)
#define SOCKET int
#endif

#define SOCKET_ERROR	 (-1)
#define LPSOCKADDR sockaddr*
#define MAX_RESPONDERS 65535

#ifndef ERESTART
#define ERESTART 99
#endif

static SOCKET listener_socket = INVALID_SOCKET;
static int listener_port = -1;
static size_t logger_id = 0;

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
static int addrlen;
#else
static socklen_t addrlen;
#endif

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
static CRITICAL_SECTION socket_lock;
#else
static pthread_mutex_t socket_lock;
#endif

typedef struct responder_thread_params
{
    #if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
        HANDLE thread;
    #else
        pthread_t thread;
    #endif
    void* responder;
}responder_thread_params;

typedef struct client_node
{
    char node_name[32];
    void* responder;
}client_node;

static void* client_node_array[MAX_RESPONDERS];

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
static DWORD WINAPI responder_run(void* responder_thread_params);
#else
static void* responder_run(void* responder_thread_params);
#endif

static void on_signal_received(SignalType type);
static bool payload_handle_protocol(payload* message, void* vptr_responder);
static bool payload_receive(payload* message, void* vptr_responder);
static bool payload_send(payload* message, void* vptr_responder);
static bool payload_broadcast_registration(const char* node_name);
static bool payload_broadcast_deregistration(const char* node_name);
static bool payload_send_nodelist(client_node* node);
static void print_nodes();

bool broker_initialize(char* appname, int port)
{
    listener_port = port;

    logger_id = logger_allocate(10, appname, NULL);
    logger_start_logging(logger_id);

    signals_register_callback(on_signal_received);
    signals_initialize_handlers();

    #if defined(_WIN32) || defined(WIN32)
        WSACleanup();
        WSADATA WSData;
        long nRc = WSAStartup(0x0202, &WSData);
        if (nRc != 0)
        {
            return false;
        }
        if (WSData.wVersion != 0x0202)
        {
            WSACleanup();
            return false;
        }
    #endif

    #if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
    if (!InitializeCriticalSectionAndSpinCount(&socket_lock, 0x00000400))
    {
        return false;
    }
    #else
        pthread_mutex_init(&socket_lock, NULL);
    #endif

    return true;
}

bool broker_run()
{
    listener_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in bindAddr;

    memset((void*)&bindAddr, 0, sizeof(struct sockaddr_in));

    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(listener_port);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listener_socket, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
    {
        return BindFailed;
    }

    if(listen(listener_socket,5) == SOCKET_ERROR)
    {
        return ListenFailed;
    }

    while(true)
    {
        struct sockaddr remotehostaddr;
        memset((void*)&remotehostaddr, 0, sizeof(remotehostaddr));
        addrlen = sizeof(remotehostaddr);

        SOCKET client_sock = accept(listener_socket,&remotehostaddr,&addrlen);
        if(client_sock != INVALID_SOCKET)
        {
            struct responder_thread_params* params = (responder_thread_params*)calloc(1, sizeof (struct responder_thread_params));

            if (params)
            {
                params->responder = responder_assign_socket(params->responder, client_sock);

                #if defined(_WIN32) || defined(WIN32) || defined(_WIN64)

                    int thread_id = 0;
                    params->thread = CreateThread(NULL, 0, &responder_run, (LPVOID)params, CREATE_SUSPENDED, &thread_id);
                    if (params->thread == NULL)
                    {
                        return false;
                    }
                    ResumeThread(params->thread);

                #else

                    pthread_attr_t pthread_attr;
                    memset(&pthread_attr, 0, sizeof(pthread_attr_t));
                    // default threading attributes
                    pthread_attr_init(&pthread_attr);
                    // allow a thread to exit cleanly without a join
                    pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
                    if (pthread_create(&params->thread, &pthread_attr, responder_run, params) != 0)
                    {
                        responder_close_socket(params->responder);
                        free(params);
                        continue;
                    }

                #endif
            }
            else
            {
                shutdown(client_sock, 2);
                #if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
                    closesocket(client_sock);
                #else
                    close(client_sock);
                #endif
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

    #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
        pthread_mutex_destroy(&socket_lock);
        close(listener_socket);
   #else
        DeleteCriticalSection(&socket_lock);
        closesocket(listener_socket);
#endif

    #if defined(_WIN32) || defined(WIN32)
            WSACleanup();
    #endif
}

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)

DWORD WINAPI responder_run(void*  responder_thread_params)
{
    struct responder_thread_params* params = (struct responder_thread_params*)responder_thread_params;

    if (params == NULL)
    {
        ExitThread(0);
        return 0;
    }

    if (params->responder == NULL)
    {
        ExitThread(0);
        free(responder_thread_params);
        return 0;
    }

    SOCKET current_socket = responder_get_socket(params->responder);
    bool ret = true;

    while (true)
    {
        payload client_payload = { 0 };

        if (payload_receive(&client_payload, params->responder))
        {
            bool ret = payload_handle_protocol(&client_payload, params->responder);

            if (client_payload.data_size > 0 && client_payload.data != NULL)
            {
                free(client_payload.data);
            }

            if (!ret)
            {
                break;
            }
        }
        else
        {
            #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                        pthread_mutex_lock(&socket_lock);
            #else
                        EnterCriticalSection(&socket_lock);
            #endif

            char node_name[32] = { 0 };

            strcpy(node_name, ((client_node*)client_node_array[current_socket])->node_name);

            responder_close_socket(params->responder);
            client_node_array[current_socket] = NULL;

            payload_broadcast_deregistration(node_name);

            #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                        pthread_mutex_unlock(&socket_lock);
            #else
                        LeaveCriticalSection(&socket_lock);
            #endif

            print_nodes();

            break;
        }
    }

    free(params);

    ExitThread(0);
    return 0;
}

#else

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

    SOCKET current_socket = responder_get_socket(params->responder);
    bool ret = true;

    while(true)
    {
        payload client_payload = {0};

        if(payload_receive(&client_payload, params->responder))
        {
            ret = payload_handle_protocol(&client_payload, params->responder);

            if (client_payload.data_size > 0 && client_payload.data != NULL)
            {
                free(client_payload.data);
            }

            if (!ret)
            {
                break;
            }
        }
        else
        {
            #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                        pthread_mutex_lock(&socket_lock);
            #else
                        EnterCriticalSection(&socket_lock);
            #endif

            char node_name[32] = { 0 };

            strcpy(node_name, ((client_node*)client_node_array[current_socket])->node_name);

            responder_close_socket(params->responder);
            client_node_array[current_socket] = NULL;

            payload_broadcast_deregistration(node_name);

            #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                        pthread_mutex_unlock(&socket_lock);
            #else
                        LeaveCriticalSection(&socket_lock);
            #endif

            print_nodes();

            break;
        }
    }

    pthread_cancel(params->thread);
    free(params);
    pthread_exit(NULL);
}

#endif

bool payload_handle_protocol(payload* message, void* vptr_responder)
{
    struct responder* responder_ptr = (struct responder*)vptr_responder;

    if(!responder_ptr)
    {
        return false;
    }

    int sender_socket = responder_get_socket(responder_ptr);

    // Event => Registration
    if(message->payload_type == PAYLOAD_TYPE_EVENT && message->payload_sub_type == PAYLOAD_SUB_TYPE_REGISTER)
    {
        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                pthread_mutex_lock(&socket_lock);
        #else
                EnterCriticalSection(&socket_lock);
        #endif

        // Make sure that the registration request is from a new node
        client_node* new_node = client_node_array[sender_socket];

        if(new_node == NULL)
        {
            new_node = (struct client_node*)calloc(1, sizeof (struct client_node));

            if (new_node)
            {
                strcpy(new_node->node_name, message->sender);
                new_node->responder = responder_ptr;
                client_node_array[sender_socket] = new_node;

                // Send Node Online event to all other nodes
                payload_broadcast_registration(message->sender);

                // Send node list payload for the new node
                payload_send_nodelist(new_node);
            }
        }

        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                pthread_mutex_unlock(&socket_lock);
        #else
                LeaveCriticalSection(&socket_lock);
        #endif

        print_nodes();
        return true;
    }

    // Event => Deregistration
    if(message->payload_type == PAYLOAD_TYPE_EVENT && message->payload_sub_type == PAYLOAD_SUB_TYPE_DEREGISTER)
    {
        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                pthread_mutex_lock(&socket_lock);
        #else
                EnterCriticalSection(&socket_lock);
        #endif

        client_node* old_node = client_node_array[sender_socket];

        // If we have the node in our array
        if(old_node != NULL)
        {
            payload_broadcast_deregistration(old_node->node_name);

            // Now remove the old node from the array
            responder_close_socket(old_node->responder);
            client_node_array[sender_socket] = NULL;
        }

        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                pthread_mutex_unlock(&socket_lock);
        #else
                LeaveCriticalSection(&socket_lock);
        #endif
                
        print_nodes();
        return false;
    }

    //Loopback
    if(message->payload_type == PAYLOAD_TYPE_DATA && message->payload_sub_type == PAYLOAD_SUB_TYPE_LOOPBACK)
    {
        payload_send(message, responder_ptr);
        return true;
    }

    // All other payload types that carry trailing data buffers
    if(message->payload_type == PAYLOAD_TYPE_DATA || message->payload_type == PAYLOAD_TYPE_REQUEST || message->payload_type == PAYLOAD_TYPE_RESPONSE || message->payload_type ==  PAYLOAD_TYPE_EVENT)
    {
        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                pthread_mutex_lock(&socket_lock);
        #else
                EnterCriticalSection(&socket_lock);
        #endif

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

        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
                pthread_mutex_unlock(&socket_lock);
        #else
                LeaveCriticalSection(&socket_lock);
        #endif

        return true;
    }

    return true;
}

bool payload_broadcast_registration(const char* node_name)
{
    for (size_t index = 0; index < MAX_RESPONDERS; index++)
    {
        if (client_node_array[index] == NULL)
        {
            continue;
        }

        client_node* ptr = client_node_array[index];

        if (strcmp(ptr->node_name, node_name) == 0)
        {
            continue;
        }

        struct payload node_online_message = { 0 };
        node_online_message.payload_type = PAYLOAD_TYPE_EVENT;
        node_online_message.payload_sub_type = PAYLOAD_SUB_TYPE_NODE_ONLINE;
        node_online_message.payload_data_type = PAYLOAD_DATA_TYPE_TEXT;
        node_online_message.payload_id = 0;
        strcpy(node_online_message.receipient, ptr->node_name);
        strcpy(node_online_message.sender, "MessageBus");
        node_online_message.data_size = strlen(node_name);

        node_online_message.data = calloc(1, node_online_message.data_size + (size_t)1);
        if (node_online_message.data)
        {
            strcpy(node_online_message.data, node_name);
            payload_send(&node_online_message, ptr->responder);
            free(node_online_message.data);
            return true;
        }
    }

    return false;
}

bool payload_broadcast_deregistration(const char* node_name)
{
    // Send Node Offline event to all other nodes
    for (size_t index = 0; index < MAX_RESPONDERS; index++)
    {
        if (client_node_array[index] == NULL)
        {
            continue;
        }

        client_node* ptr = client_node_array[index];

        if (strcmp(ptr->node_name, node_name) == 0)
        {
            continue;
        }

        struct payload node_offline_message;
        node_offline_message.payload_type = PAYLOAD_TYPE_EVENT;
        node_offline_message.payload_sub_type = PAYLOAD_SUB_TYPE_NODE_OFFLINE;
        node_offline_message.payload_data_type = PAYLOAD_DATA_TYPE_TEXT;
        node_offline_message.payload_id = 0;
        strcpy(node_offline_message.receipient, ptr->node_name);
        strcpy(node_offline_message.sender, "MessageBus");
        node_offline_message.data_size = strlen(node_name);
        node_offline_message.data = calloc(1, node_offline_message.data_size + (size_t)1);

        if (node_offline_message.data)
        {
            strcpy(node_offline_message.data, node_name);
            payload_send(&node_offline_message, ptr->responder);
            free(node_offline_message.data);
            return true;
        }
    }

    return false;
}

bool payload_send_nodelist(client_node* node)
{
    struct payload node_list_message = { 0 };
    node_list_message.payload_type = PAYLOAD_TYPE_EVENT;
    node_list_message.payload_sub_type = PAYLOAD_SUB_TYPE_NODELIST;
    node_list_message.payload_data_type = PAYLOAD_DATA_TYPE_TEXT;
    node_list_message.payload_id = 0;
    strcpy(node_list_message.receipient, node->node_name);
    strcpy(node_list_message.sender, "MessageBus");

    for (size_t index = 0; index < MAX_RESPONDERS; index++)
    {
        if (client_node_array[index] == NULL)
        {
            continue;
        }

        client_node* ptr = client_node_array[index];

        if (strcmp(ptr->node_name, node->node_name) == 0)
        {
            continue;
        }

        if (node_list_message.data == NULL)
        {
            size_t len = strlen(ptr->node_name);
            node_list_message.data = calloc(1, len + 2);

            if (node_list_message.data)
            {
                strcpy(node_list_message.data, ptr->node_name);
                node_list_message.data[len] = ',';
            }
        }
        else
        {
            size_t len = strlen(ptr->node_name) + strlen(node_list_message.data);

            char* temp_buffer = calloc(1, len + (size_t)2);
            if (temp_buffer)
            {
                strcpy(temp_buffer, (char*)node_list_message.data);
                strcat(temp_buffer, ptr->node_name);
                temp_buffer[len] = ',';

                free(node_list_message.data);
                node_list_message.data = temp_buffer;
            }
        }
    }

    //Out of loop

    if (node_list_message.data)
    {
        node_list_message.data[strlen(node_list_message.data) - 1] = 0;
        node_list_message.data_size = strlen(node_list_message.data);

        // Now send Node List event to the newly added node
        payload_send(&node_list_message, node->responder);
        free(node_list_message.data);
        return true;
    }
    
    return false;
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

    if(!responder_receive_buffer(responder_ptr, (char*)&message, sizeof (struct payload) - sizeof(char*), false))
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

    if(!responder_send_buffer(responder_ptr, (const char*)message, sizeof (struct payload) - sizeof (char*)))
    {
        return false;
    }

    if(message->data_size > 0 && message->data != NULL)
    {
        if(!responder_send_buffer(responder_ptr, message->data, message->data_size))
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

void print_nodes()
{
    printf("\n\nPRINT NODES ++\n");
    for (size_t index = 0; index < MAX_RESPONDERS; index++)
    {
        client_node* ptr = client_node_array[index];

        if (ptr != NULL)
        {
            printf("%s\n", ptr->node_name);
        }
    }
    printf("PRINT NODES --\n\n");
}
