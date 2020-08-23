/*

Copyright (c) 2020, CIMCON Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, is allowed only with prior permission from CIMCON Automation

*/

#ifndef MESSAGE_BROKER_C
#define MESSAGE_BROKER_C

#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum RunState
{
    Running =0,
    NormalExit = 1,
    BindFailed = -1,
    ListenFailed = -2,
    SignalReceived = 3
}RunState;

bool broker_initialize(char* appname, int port);
bool broker_run();
void broker_stop();

#ifdef __cplusplus
}
#endif

#endif
