/*

Copyright (c) 2020, CIMCON Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, is allowed only with prior permission from CIMCON Automation

*/

#ifndef LIB_MESSAGE_BUS_C
#define LIB_MESSAGE_BUS_C

#include <stdint.h>
#include <stdbool.h>

// Shared libary load/unload handlers
extern __attribute__((destructor)) void library_load();
extern __attribute__((destructor)) void library_unload();

// Structures and types
typedef enum PayloadType
{
    Data='D',
    Event='E',
    Request='Q',
    Response='R'
}PayloadType;

typedef enum MessageType
{
    ConfigurationRequest='C',
    PolicyRequest='P',
    ConfigurationResponse='c',
    PolicyResponse='p',
    Register='G',
    DeRegister='D',
    NodeOnline='N',
    NodeOffline='F',
    NodeList='L',
    LoopBack='K',
    UserData='U'
}MessageType;

/*
class MessageCallback
{
    public:
        MessageCallback() {}
        virtual ~MessageCallback() {}
        virtual void OnNodeOnline(const std::string &nodename) = 0;
        virtual void OnNodeOffline(const std::string &nodename) = 0;
        virtual void OnData(const std::string &nodename, const std::string &messagebuffer) = 0;
        virtual void OnEvent(const std::string &nodename, const std::string &messagebuffer) = 0;
        virtual void OnRequest(const std::string &nodename, const std::string &messagebuffer) = 0;
        virtual void OnResponse(const std::string &nodename, const std::string &messagebuffer) = 0;
};

class Messenger
{
    public:
        Messenger();
        virtual ~Messenger();
        // Session management
        bool Initialize();
        bool Open(MessageCallback *callback);
        bool Close();

        // Node management
        bool Register();
        bool DeRegister();
        std::list<std::string>* GetPeerlist();

        // Messaging
        bool Send(const std::string &nodename, MessageType messagetype, const std::string &messagebuffer);        
};
*/

typedef void(*messabus_bus_callback)(const char* node_name, PayloadType ptype, MessageType mtype, const char* messagebuffer);

extern __attribute__((visibility("default"))) bool message_bus_initialize(void** pptr, messabus_bus_callback callback);
extern __attribute__((visibility("default"))) bool message_bus_open(void* ptr);
extern __attribute__((visibility("default"))) bool message_bus_close(void* ptr);
extern __attribute__((visibility("default"))) bool message_bus_register(void* ptr);
extern __attribute__((visibility("default"))) bool message_bus_deregister(void* ptr);
extern __attribute__((visibility("default"))) bool message_bus_send(void* ptr, const char* node_name, PayloadType ptype, MessageType mtype, const char* messagebuffer, long buffersize);
extern __attribute__((visibility("default"))) bool message_bus_has_node(void* ptr, const char* node_name);

#endif
