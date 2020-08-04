#include "SignalHandler.h"
#include "Logger.h"
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#define SIG_WIN_DUMMY 9999
#define SIGHUP SIG_WIN_DUMMY+1
#define SIGQUIT SIG_WIN_DUMMY+2
#define SIGTRAP SIG_WIN_DUMMY+3
#define SIGBUS SIG_WIN_DUMMY+4
#define SIGPIPE SIG_WIN_DUMMY+5
#define SIGUSR1 SIG_WIN_DUMMY+6
#define SIGUSR2 SIG_WIN_DUMMY+7
#define SIGCHLD SIG_WIN_DUMMY+8
#define SIGKILL SIG_WIN_DUMMY+9
#define SIGSTOP SIG_WIN_DUMMY+10
#define SIGALRM SIG_WIN_DUMMY+11
#define SIGTSTP SIG_WIN_DUMMY+12
#define SIGCONT SIG_WIN_DUMMY+13
#else
#include <sys/wait.h>
#endif

// + BSD specific starts
#ifndef SIGSTKFLT
#define SIGSTKFLT 16
#endif
// + BSD specific ends

static int signal_numbers[] = {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGSEGV, SIGPIPE, SIGTERM, SIGSTKFLT, SIGUSR1, SIGUSR2, SIGCHLD};
static const char *signal_names[] = {"SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGSEGV", "SIGPIPE", "SIGTERM", "SIGSTKFLT", "SIGUSR1", "SIGUSR2", "SIGCHLD"};

static char signal_name_string[16]={0};

static signal_callback callback_ptr = NULL;

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
void signal_handler_internal(int signum);
#else
void signal_handler_internal(int signum, siginfo_t *siginfo, void *context);
#endif

bool signals_is_shutdownsignal(const int signum)
{
    int ctr = 0;

    bool found = false;

    for(ctr = 0; ctr < 15; ctr++)
    {
        if(signal_numbers[ctr] == signum)
        {
            found = true;
            break;
        }
    }

    return found;
}

void signals_get_name(const int signum)
{
    int ctr = 0;

    memset((char*)&signal_name_string[0], 0, sizeof(signal_name_string));
    strcpy(signal_name_string, "<Not Named>");

    for(ctr = 0; ctr < 15; ctr++)
    {
        if(signal_numbers[ctr] == signum)
        {
            memset((char*)&signal_name_string[0], 0, sizeof(signal_name_string));
            strcpy(signal_name_string, signal_names[ctr]);
            break;
        }
    }
}

void signals_register_callback(signal_callback callback_func)
{
    callback_ptr = callback_func;
}

void signals_initialize_handlers()
{
    for(int signum = 1; signum < 32; signum++)
    {
        #if defined(_WIN32) || defined(WIN32) || defined(_WIN64)

        signals_get_name(signum);
        signal(signum, signal_handler_internal);

        #else

        signals_get_name(signum);
        struct sigaction act;
        memset(&act, '\0', sizeof(act));
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = &signal_handler_internal;
        sigaction(signum, &act, NULL);

        #endif
    }

}

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)

void signal_handler_internal(int signum)
{
    switch (signum)
    {
    case SIGKILL:
    case SIGSTOP:
    case SIGINT:
    case SIGQUIT:
    case SIGILL:
    case SIGTRAP:
    case SIGABRT:
    case SIGBUS:
    case SIGFPE:
    case SIGSEGV:
    case SIGPIPE:
    case SIGTERM:
    case SIGSTKFLT:
    default:
    {
        callback_ptr(Shutdown);
        break;
    }
    case SIGALRM:
    {
        callback_ptr(Alarm);
        break;
    }
    case SIGTSTP:
    {
        callback_ptr(Suspend);
        break;
    }
    case SIGCONT:
    {
        callback_ptr(Resume);
        break;
    }
    case SIGHUP:
    {
        callback_ptr(Reset);
        break;
    }
    case SIGCHLD:
    {
        callback_ptr(ChildExit);
        break;
    }
    case SIGUSR1:
    {
        callback_ptr(Userdefined1);
        break;
    }
    case SIGUSR2:
    {
        callback_ptr(Userdefined2);
        break;
    }
    }
}

#else

void signal_handler_internal(int signum, siginfo_t *siginfo, void *context)
{
    switch(signum)
    {
        case SIGKILL:
        case SIGSTOP:
        case SIGINT:
        case SIGQUIT:
        case SIGILL:
        case SIGTRAP:
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
        case SIGPIPE:
        case SIGTERM:
        case SIGSTKFLT:
        default:
        {
            callback_ptr(Shutdown);
            break;
        }
        case SIGALRM:
        {
            callback_ptr(Alarm);
            break;
        }
        case SIGTSTP:
        {
            callback_ptr(Suspend);
            break;
        }
        case SIGCONT:
        {
            callback_ptr(Resume);
            break;
        }
        case SIGHUP:
        {
            callback_ptr(Reset);
            break;
        }
        case SIGCHLD:
        {
            callback_ptr(ChildExit);
            break;
        }
        case SIGUSR1:
        {
            callback_ptr(Userdefined1);
            break;
        }
        case SIGUSR2:
        {
            callback_ptr(Userdefined2);
            break;
        }
    }
}
#endif