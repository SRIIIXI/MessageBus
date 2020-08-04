#ifndef SIGNAL_HANDLER_C
#define SIGNAL_HANDLER_C

#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) || defined (WIN32) || defined (_WIN64)
#define LIBRARY_EXPORT __declspec(dllexport)
#define LIBRARY_ENTRY
#define LIBRARY_EXIT 
#else
#define LIBRARY_EXPORT LIBRARY_EXPORT
#define LIBRARY_ENTRY __attribute__((constructor))
#define LIBRARY_EXIT __attribute__((destructor))
#endif 

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

extern LIBRARY_EXPORT void signals_initialize_handlers();
extern LIBRARY_EXPORT void signals_register_callback(signal_callback callback_func);
extern LIBRARY_EXPORT bool signals_is_shutdownsignal(const int signum);
extern LIBRARY_EXPORT void signals_get_name(const int signum);

#endif
