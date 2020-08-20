/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SignalHandler.h"
#include "Logger.h"
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include <sys/wait.h>

// + BSD specific starts
#ifndef SIGSTKFLT
#define SIGSTKFLT 16
#endif
// + BSD specific ends

static int signal_numbers[] = {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGSEGV, SIGPIPE, SIGTERM, SIGSTKFLT, SIGUSR1, SIGUSR2, SIGCHLD};
static const char *signal_names[] = {"SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGSEGV", "SIGPIPE", "SIGTERM", "SIGSTKFLT", "SIGUSR1", "SIGUSR2", "SIGCHLD"};

static char signal_name_string[16]={0};

static signal_callback callback_ptr = NULL;
void signal_handler_internal(int signum, siginfo_t *siginfo, void *context);

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
        signals_get_name(signum);
        struct sigaction act;
        memset(&act, '\0', sizeof(act));
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = &signal_handler_internal;
        sigaction(signum, &act, NULL);
    }

}

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
