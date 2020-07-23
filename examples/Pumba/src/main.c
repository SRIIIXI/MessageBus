#include "MessageBusClient.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void network_event(const char* node_name, PayloadType ptype, MessageType mtype, const char* messagebuffer);

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
    int snooze_time = 0;

    while(snooze_time < 600)
    {
        if(message_bus_has_node(message_bus, "Timon"))
        {
            message_bus_send(message_bus, "Timon", Data, UserData, str, strlen(str));
        }

        sleep(5);
        snooze_time += 5;
    }

    message_bus_register(message_bus);
    message_bus_close(message_bus);

    return 0;
}

void network_event(const char* node_name, PayloadType ptype, MessageType mtype, const char* messagebuffer)
{
    printf("%s %c %c %s\n", node_name, ptype, mtype, messagebuffer);
}
