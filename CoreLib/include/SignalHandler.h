#ifndef SIGNAL_HANDLER_C
#define SIGNAL_HANDLER_C

#include <stdbool.h>

typedef enum SignalType
{
    Suspend=0,
    Resume=1,
    Shutdown=2,
    Alarm=3,
    Reset=4,
    ChildExit=5,
    Userdefined1=6,
    Userdefined2=7
}SignalType;

typedef void(*signal_callback)(SignalType stype);

extern __attribute__((visibility("default"))) void signals_initialize_handlers();
extern __attribute__((visibility("default"))) void signals_register_callback(signal_callback callback_func);
extern __attribute__((visibility("default"))) bool signals_is_shutdownsignal(const int signum);
extern __attribute__((visibility("default"))) void signals_get_name(const int signum);

#endif
