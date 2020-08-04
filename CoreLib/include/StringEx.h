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

#ifndef STRING_EX_C
#define STRING_EX_C

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

#if defined(_WIN32) || defined (WIN32) || defined (_WIN64)
#define LIBRARY_EXPORT __declspec(dllexport)
#define LIBRARY_ENTRY
#define LIBRARY_EXIT 
#else
#define LIBRARY_EXPORT __attribute__((visibility("default")))
#define LIBRARY_ENTRY __attribute__((constructor))
#define LIBRARY_EXIT __attribute__((destructor))
#endif 

extern LIBRARY_EXPORT wchar_t* strtowstr(const char* str);
extern LIBRARY_EXPORT char* strfromwstr(const wchar_t* wstr);

extern LIBRARY_EXPORT char* strfromint(size_t num);
extern LIBRARY_EXPORT char* strfromdouble(double num);

extern LIBRARY_EXPORT char* strreverse(char* ptr);
extern LIBRARY_EXPORT char* strsegmentreverse(char* str, size_t start, size_t term);

extern LIBRARY_EXPORT long long strindexofsubstr(char* str, const char* substr);
extern LIBRARY_EXPORT long long strindexofchar(char* str, const char ch);

extern LIBRARY_EXPORT size_t strcountsubstr(char* str, const char* substr);
extern LIBRARY_EXPORT size_t strcountchar(char* str, const char ch);

extern LIBRARY_EXPORT char* strtolower(char* str);
extern LIBRARY_EXPORT char* strtoupper(char* str);

extern LIBRARY_EXPORT char* strlefttrim(char* str);
extern LIBRARY_EXPORT char* strrighttrim(char* str);
extern LIBRARY_EXPORT char* stralltrim(char* str);

extern LIBRARY_EXPORT char* strremsubstrfirst(char* str, const char* substr);
extern LIBRARY_EXPORT char* strremsubstrall(char* str, const char* substr);
extern LIBRARY_EXPORT char* strremsubstrat(char* str, size_t pos, size_t len);

extern LIBRARY_EXPORT char* strremcharfirst(char* str, const char oldchar);
extern LIBRARY_EXPORT char* strremcharall(char* str, const char oldchar);
extern LIBRARY_EXPORT char* strremcharat(char* str, size_t pos);

extern LIBRARY_EXPORT char* strrepsubstrfirst(char* str, const char* oldsubstr, const char* newsubstr);
extern LIBRARY_EXPORT char* strrepsubstrall(char* str, const char* oldsubstr, const char* newsubstr);

extern LIBRARY_EXPORT char* strrepcharfirst(char* str, const char oldchar, const char newchar);
extern LIBRARY_EXPORT char* strrepcharall(char* str, const char oldchar, const char newchar);
extern LIBRARY_EXPORT char* strrepcharat(char* str, const char newchar, size_t pos);

extern LIBRARY_EXPORT void strsplitkeyvalue(const char* str, const char* delimiter, char **key, char **value);
extern LIBRARY_EXPORT char** strsplitsubstr(const char* str, const char* delimiter, size_t *numsubstr);
extern LIBRARY_EXPORT char** strsplitchar(const char* str, const char delimiter, size_t *numsubstr);
extern LIBRARY_EXPORT char* strjoinwithsubstr(const char **strlist, const char* delimiter);
extern LIBRARY_EXPORT char* strjoinwithchar(const char** strlist, const char delimiter);
extern LIBRARY_EXPORT void  strfreelist(char** strlist, size_t numsubstr);

#endif
