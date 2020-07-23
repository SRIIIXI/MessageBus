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

#ifndef LOGGER_C
#define LOGGER_C

#include <stddef.h>

typedef enum LogLevel
{
	LOG_INFO = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_CRITICAL = 3,
	LOG_PANIC = 4
}LogLevel;

extern __attribute__((visibility("default"))) size_t  logger_allocate_default();
extern __attribute__((visibility("default"))) size_t  logger_allocate(size_t flszmb, const char* mname, const char* dirpath);
extern __attribute__((visibility("default"))) void    logger_release(size_t loggerid);
extern __attribute__((visibility("default"))) void    logger_start_logging(size_t loggerid);
extern __attribute__((visibility("default"))) void    logger_stop_logging(size_t loggerid);
extern __attribute__((visibility("default"))) void    logger_write(size_t loggerid, const char* logentry, LogLevel llevel, const char* func, const char* file, int line);
extern __attribute__((visibility("default"))) size_t  logger_get_instance();

#define WriteLog(id, str, level) \
    logger_write(id, str, level, __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define WriteLogNormal(id, str) \
    logger_write(id, str, LOG_INFO, __PRETTY_FUNCTION__, __FILE__, __LINE__);

#endif

