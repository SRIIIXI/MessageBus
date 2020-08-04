#include "MessageBusClient.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif

void network_event(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long *payload_id);

int main(int argc, char* argv[])
{
    void* message_bus = NULL;

    if(!message_bus_initialize(&message_bus, network_event))
    {
        return -1;
    }

    if(!message_bus_open(message_bus))
    {
        return -1;
    }

    if(!message_bus_register(message_bus))
    {
        return -1;
    }

    char str[] = "Hello Timon! I'm Pumba";
    long payload_id = 0;
    int snooze_time = 0;

    message_bus_send(message_bus, "Pumba", Data, LoopBack, Text, str, strlen(str), &payload_id);

    while(snooze_time < 600)
    {
        if(message_bus_has_node(message_bus, "Timon"))
        {
            long payload_id = 0;
            message_bus_send(message_bus, "Timon", Data, UserData, Text, str, strlen(str), &payload_id);
        }

        #if !defined(_WIN32) && !defined(WIN32) && !defined(_WIN64)
        sleep(5);
        #else
        Sleep(500);
        #endif

        snooze_time += 5;
    }

    message_bus_deregister(message_bus);
    message_bus_close(message_bus);

    return 0;
}

void network_event(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long *payload_id)
{
    printf("%s %c %c %s\n", node_name, ptype, mtype, messagebuffer);
}
