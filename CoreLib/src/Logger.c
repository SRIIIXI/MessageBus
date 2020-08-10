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

#include "Logger.h"
#include "Directory.h"
#include "File.h"
#include "StringEx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#include <Windows.h>
#include <process.h>
#include <direct.h>
#define pid_t int
#define getpid _getpid
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#define END_OF_LINE "\n"
#define MAX_LOGGERS 512

static char log_level_names[5][16] = {"Information", "Error", "Warning", "Critical", "Panic"};

void normalize_function_name(char* func_name);

#pragma pack(1)
typedef struct Logger
{
    size_t LogFileSizeMB;
    char FileName[1025];
    FILE* FileHandle;
    bool IsOpen;
    int PID;
}Logger;

static Logger *loggers[MAX_LOGGERS] = {0};

size_t	logger_allocate_default()
{
    char owner[33] = {0};
    sprintf(owner, "%d", getpid());

    return logger_allocate(10, owner, NULL);
}

size_t	logger_allocate(size_t flszmb, const char* mname, const char* dirpath)
{
    size_t index = 0;
    while(index < MAX_LOGGERS)
    {
        if(loggers[index] == NULL)
        {
            break;
        }
        index++;
    }

    loggers[index] = (Logger*)calloc(1, sizeof(Logger));

    if(loggers[index] == NULL)
    {
        return SIZE_MAX;
    }

    loggers[index]->PID = getpid();

    loggers[index]->IsOpen = false;
    loggers[index]->FileHandle = NULL;

    if(flszmb < 1 || flszmb > 10)
    {
        flszmb = 10;
    }

    loggers[index]->LogFileSizeMB = flszmb;

    if(dirpath != NULL)
    {
        strcat(loggers[index]->FileName, dirpath);

        if(dirpath[strlen(dirpath) - 1] != '/')
        {
            strcat(loggers[index]->FileName, "/");
        }
    }
    else
    {
        char wd_path[1025] = { 0 };
        int wd_len = 1024;
        getcwd(wd_path, wd_len);
        char* parent_dir = dir_get_parent_directory(wd_path);
        strcat(loggers[index]->FileName, parent_dir);
        strcat(loggers[index]->FileName, "log/");
        free(parent_dir);
    }

    if(!dir_is_exists(loggers[index]->FileName))
    {
        dir_create_directory(loggers[index]->FileName);
    }

    if(mname != NULL)
    {
        if(strcountchar(mname, '/') > 0)
        {
            char* base_name = file_get_basename(mname);
            strcat(loggers[index]->FileName, base_name);
            free(base_name);
        }
        else
        {
            strcat(loggers[index]->FileName, mname);
        }
    }
    else
    {
        char owner[33] = { 0 };
        sprintf(owner, "%d", getpid());
        strcat(loggers[index]->FileName, owner);
    }

    strcat(loggers[index]->FileName, ".log");

    return (index+1);
}

void logger_release(size_t loggerid)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return;
    }

    if(loggers[loggerid] == NULL)
    {
        return;
    }

    if(loggers[loggerid]->IsOpen)
    {
        fflush(loggers[loggerid]->FileHandle);
        fclose(loggers[loggerid]->FileHandle);
    }

    free(loggers[loggerid]);
}

void logger_start_logging(size_t loggerid)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return;
    }

    if(loggers[loggerid] == NULL)
    {
        return;
    }

   loggers[loggerid]->FileHandle = fopen(loggers[loggerid]->FileName, "w");

   if(loggers[loggerid]->FileHandle != NULL)
   {
       loggers[loggerid]->IsOpen = true;
   }

   return;
}

void logger_stop_logging(size_t loggerid)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return;
    }

    if(loggers[loggerid] == NULL)
    {
        return;
    }

    if(loggers[loggerid]->IsOpen)
    {
        fflush(loggers[loggerid]->FileHandle);
        fclose(loggers[loggerid]->FileHandle);
        loggers[loggerid]->IsOpen = false;
    }
}

void logger_write(size_t loggerid, const char* logentry, LogLevel llevel, const char* func, const char* file, int line)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return;
    }

    if(loggers[loggerid] == NULL)
    {
        return;
    }

    if(loggers[loggerid]->FileHandle == NULL || loggers[loggerid]->IsOpen == false)
    {
        return;
    }

    // Check the file size
    size_t sz = (size_t)ftell(loggers[loggerid]->FileHandle);

    // If it exceeds the set size
    if(sz*1024*1024 >= loggers[loggerid]->LogFileSizeMB)
    {
        // Stop logging
        logger_stop_logging(loggerid);

        // Rename the file
        char old_log_filename[1025] = {0};
        strcat(old_log_filename, loggers[loggerid]->FileName);
        strcat(old_log_filename, ".old");

        rename(loggers[loggerid]->FileName, old_log_filename);

        // Reopen the log file with original name
        logger_start_logging(loggerid);
    }

    normalize_function_name((char*)func);

    time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

    // Timestamp
    fprintf(loggers[loggerid]->FileHandle, "%d-%d-%d %d:%d:%d\t",
             tmp->tm_mday, tmp->tm_mon, tmp->tm_year,
             tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    // Level
    fprintf(loggers[loggerid]->FileHandle, "%s\t", log_level_names[llevel]);

    // File
    char* base_file_name = file_get_basename(file);
    fprintf(loggers[loggerid]->FileHandle, "%s\t", base_file_name);
    free(base_file_name);

    // Function
    normalize_function_name(func);
    fprintf(loggers[loggerid]->FileHandle, "%s\t", func);

    // Message
    fprintf(loggers[loggerid]->FileHandle, "%s", logentry);

    // End of line
    fprintf(loggers[loggerid]->FileHandle, END_OF_LINE);

    // Flush th contents
    fflush(loggers[loggerid]->FileHandle);
}

size_t logger_get_instance()
{
    size_t index = 0;

    for(index = 0; index < MAX_LOGGERS; index++)
    {
        if(loggers[index] != NULL)
        {
            if(loggers[index]->PID == getpid())
            {
                return (index+1);
            }
        }
    }

    return SIZE_MAX;
}

void normalize_function_name(char* func_name)
{
    
}
