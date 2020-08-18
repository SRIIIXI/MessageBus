#include "MessageBusClient.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void network_event(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long payload_id);

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
    printf("I am Pumba. I am heavy and fat. I use a C++ based IPC\n");

    char str[] = "Hello Timon! I'm Pumba";
    long payload_id = 0;
    int snooze_time = 0;

    message_bus_send(message_bus, "Pumba", Data, LoopBack, Text, str, strlen(str), &payload_id);

    while(snooze_time < 600)
    {
        long node_index = -1;
        node_index = message_bus_has_node(message_bus, "Timon");
        if(node_index > -1)
        {
            long payload_id = 0;
            char* node_full_name = message_bus_node_fullname(message_bus, node_index);
            message_bus_send(message_bus, node_full_name, Data, UserData, Text, str, strlen(str), &payload_id);
        }

        sleep(5);
        snooze_time += 5;
    }

    message_bus_deregister(message_bus);
    message_bus_close(message_bus);

    return 0;
}

void network_event(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long payload_id)
{
    printf("%s %c %c %c %ld %s %ld\n", node_name, ptype, mtype, dtype, buffersize, messagebuffer, payload_id);
}
