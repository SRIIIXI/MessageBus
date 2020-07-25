#ifndef MESSAGE_BROKER_C
#define MESSAGE_BROKER_C

#include <string.h>
#include <stdbool.h>

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

#endif
