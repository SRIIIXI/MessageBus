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
#include "StringList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#define END_OF_LINE "\n"
#define MAX_LOGGERS 512

static char log_level_names[5][16] = {"Information", "Error", "Warning", "Critical", "Panic"};

void normalize_function_name(char* func_name);
bool read_current_process_name(char* ptr);

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
    return logger_allocate(10, NULL);
}

size_t	logger_allocate(size_t flszmb, const char* dirpath)
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

    pid_t parent = getppid();

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
        if(parent == 0)
        {
            strcat(loggers[index]->FileName, "/var/log/");
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
    }

    if(!dir_is_exists(loggers[index]->FileName))
    {
        dir_create_directory(loggers[index]->FileName);
    }

    char temp[1024] = {0};
    read_current_process_name(&temp[0]);
    strcat(loggers[index]->FileName, temp);
    strcat(loggers[index]->FileName, ".log");

    return (index+1);
}

void logger_release(size_t loggerid)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return;
    }

    if(loggers[loggerid-1] == NULL)
    {
        return;
    }

    if(loggers[loggerid-1]->IsOpen)
    {
        fflush(loggers[loggerid-1]->FileHandle);
        fclose(loggers[loggerid-1]->FileHandle);
    }

    free(loggers[loggerid-1]);
}

bool logger_start_logging(size_t loggerid)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return false;
    }

    if(loggers[loggerid-1] == NULL)
    {
        return false;
    }

   loggers[loggerid-1]->FileHandle = fopen(loggers[loggerid-1]->FileName, "w");

   if(loggers[loggerid-1]->FileHandle != NULL)
   {
       loggers[loggerid-1]->IsOpen = true;
   }
   else
   {
       return false;
   }
   
   return true;
}

void logger_stop_logging(size_t loggerid)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return;
    }

    if(loggers[loggerid-1] == NULL)
    {
        return;
    }

    if(loggers[loggerid-1]->IsOpen)
    {
        fflush(loggers[loggerid-1]->FileHandle);
        fclose(loggers[loggerid-1]->FileHandle);
        loggers[loggerid-1]->IsOpen = false;
    }
}

bool logger_write(size_t loggerid, const char* logentry, LogLevel llevel, const char* func, const char* file, int line)
{
    if(loggerid-1 > MAX_LOGGERS-2)
    {
        return false;
    }

    if(loggers[loggerid-1] == NULL)
    {
        return false;
    }

    if(loggers[loggerid-1]->FileHandle == NULL || loggers[loggerid-1]->IsOpen == false)
    {
        return false;
    }

    // Check the file size
    size_t sz = (size_t)ftell(loggers[loggerid-1]->FileHandle);

    // If it exceeds the set size
    if(sz*1024*1024 >= loggers[loggerid-1]->LogFileSizeMB)
    {
        // Stop logging
        logger_stop_logging(loggerid-1);

        // Rename the file
        char old_log_filename[1025] = {0};
        strcat(old_log_filename, loggers[loggerid-1]->FileName);
        strcat(old_log_filename, ".old");

        rename(loggers[loggerid-1]->FileName, old_log_filename);

        // Reopen the log file with original name
        logger_start_logging(loggerid-1);
    }

    normalize_function_name((char*)func);

    time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

    // Timestamp
    fprintf(loggers[loggerid-1]->FileHandle, "%02d-%02d-%04d %02d:%02d:%02d\t",
             tmp->tm_mday, (tmp->tm_mon+1), (tmp->tm_year+1900),
             tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    // Level
    fprintf(loggers[loggerid-1]->FileHandle, "%s\t", log_level_names[llevel]);

    // File
    char* base_file_name = file_get_basename(file);
    fprintf(loggers[loggerid-1]->FileHandle, "%s\t", base_file_name);
    free(base_file_name);

    // Function
    normalize_function_name(func);
    fprintf(loggers[loggerid-1]->FileHandle, "%s\t", func);

    // Message
    fprintf(loggers[loggerid-1]->FileHandle, "%s", logentry);

    // End of line
    fprintf(loggers[loggerid-1]->FileHandle, END_OF_LINE);

    // Flush th contents
    fflush(loggers[loggerid-1]->FileHandle);

    return true;
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

bool read_current_process_name(char* ptr)
{
    if(ptr == NULL)
    {
        return false;
    }

    char buffer[1025] = {0};
    pid_t proc_id = getpid();

    sprintf(buffer, "/proc/%d/cmdline", proc_id);

    FILE* fp = fopen(buffer, "r");

    if(fp)
    {
        memset(buffer, 0, 1025);

        if(fgets(buffer, 1024, fp))
        {
            void* cmd_args = NULL;
            str_list_allocate_from_string(cmd_args, buffer, " ");

            if(cmd_args && str_list_item_count(cmd_args) > 0)
            {
                void* dir_tokens = NULL;
                str_list_allocate_from_string(dir_tokens, str_list_get_first(cmd_args), "/");

                if(dir_tokens && str_list_item_count(dir_tokens) > 0)
                {
                    strcpy(ptr, str_list_get_last(dir_tokens));
                    free(dir_tokens);
                }
                free(cmd_args);
            }
            else
            {
                void* dir_tokens = NULL;
                dir_tokens = str_list_allocate_from_string(dir_tokens, buffer, "/");

                if(dir_tokens && str_list_item_count(dir_tokens) > 0)
                {
                    strcpy(ptr, str_list_get_last(dir_tokens));
                    free(dir_tokens);
                }
            }
        }

        fclose(fp);
    }
}
