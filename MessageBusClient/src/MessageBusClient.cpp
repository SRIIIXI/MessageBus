/*

Copyright (c) 2020, CIMCON Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, is allowed only with prior permission from CIMCON Automation

*/

#include "MessageBusClient.hpp"

static IMessageCallback* _callback = nullptr;
static void NetworkEvent(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long payload_id);


MessageBusClient::MessageBusClient()
{

}

MessageBusClient::~MessageBusClient()
{

}

bool MessageBusClient::Initialize(IMessageCallback *callbackref)
{
    _callback = callbackref;
    return true;
}

bool MessageBusClient::Open()
{
    return message_bus_initialize(&message_bus, NetworkEvent);
}

bool MessageBusClient::Close()
{
    return message_bus_close(message_bus);
}

bool MessageBusClient::Register()
{
    return message_bus_register(message_bus);
}

bool MessageBusClient::DeRegister()
{
    return message_bus_deregister(message_bus);
}

bool MessageBusClient::Send(const std::string &node_name, PayloadType ptype, MessageType mtype, const std::string messagebuffer, long &payload_id)
{
    return message_bus_send(message_bus, node_name.c_str(), ptype, mtype, Text, messagebuffer.c_str(), messagebuffer.length(), &payload_id);
}

bool MessageBusClient::Send(const std::string &node_name, PayloadType ptype, MessageType mtype, DataType dtype, const std::vector<char> messagebuffer, long &payload_id)
{
    return message_bus_send(message_bus, node_name.c_str(), ptype, mtype, dtype, reinterpret_cast<const char*>(messagebuffer.data()), messagebuffer.size(), &payload_id);
}

bool MessageBusClient::HasNode(const std::string &node_name)
{
    return message_bus_has_node(message_bus, node_name.c_str());
}

std::string MessageBusClient::NodeFullname(long index)
{
    return message_bus_node_fullname(message_bus, index);
}

void NetworkEvent(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long payload_id)
{
    if (_callback)
    {
        switch (ptype)
        {
        case Event:
            {
                if (mtype == NodeOnline)
                {
                    _callback->OnNodeOnline(std::string(node_name));
                    break;
                }

                if (mtype == NodeOffline)
                {
                    _callback->OnNodeOffline(std::string(node_name));
                    break;
                }

                std::vector<char> buffer(messagebuffer, messagebuffer + buffersize);

                _callback->OnEvent(std::string(node_name), dtype, buffer);

                break;
            }
        case Request:
            {
                std::vector<char> buffer(messagebuffer, messagebuffer + buffersize);

                _callback->OnRequest(std::string(node_name), dtype, payload_id, buffer);

                break;
            }
        case Response:
            {
                std::vector<char> buffer(messagebuffer, messagebuffer + buffersize);

                _callback->OnResponse(std::string(node_name), dtype, payload_id, buffer);

                break;
            }
        case Data:
            {
                std::vector<char> buffer(messagebuffer, messagebuffer + buffersize);

                _callback->OnData(std::string(node_name), mtype, dtype, buffer);

                break;
            }
        default:
            break;
        }
    }
}

