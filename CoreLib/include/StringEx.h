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

#include <stddef.h>
#include <stdbool.h>

extern __attribute__((visibility("default"))) wchar_t* strtowstr(const char* str);
extern __attribute__((visibility("default"))) char* strfromwstr(const wchar_t* wstr);

extern __attribute__((visibility("default"))) char* strfromint(size_t num);
extern __attribute__((visibility("default"))) char* strfromdouble(double num);

extern __attribute__((visibility("default"))) char* strreverse(char* ptr);
extern __attribute__((visibility("default"))) char* strsegmentreverse(char* str, size_t start, size_t term);

extern __attribute__((visibility("default"))) long long strindexofsubstr(char* str, const char* substr);
extern __attribute__((visibility("default"))) long long strindexofchar(char* str, const char ch);

extern __attribute__((visibility("default"))) size_t strcountsubstr(char* str, const char* substr);
extern __attribute__((visibility("default"))) size_t strcountchar(char* str, const char ch);

extern __attribute__((visibility("default"))) char* strtolower(char* str);
extern __attribute__((visibility("default"))) char* strtoupper(char* str);

extern __attribute__((visibility("default"))) char* strlefttrim(char* str);
extern __attribute__((visibility("default"))) char* strrighttrim(char* str);
extern __attribute__((visibility("default"))) char* stralltrim(char* str);

extern __attribute__((visibility("default"))) char* strremsubstrfirst(char* str, const char* substr);
extern __attribute__((visibility("default"))) char* strremsubstrall(char* str, const char* substr);
extern __attribute__((visibility("default"))) char* strremsubstrat(char* str, size_t pos, size_t len);

extern __attribute__((visibility("default"))) char* strremcharfirst(char* str, const char oldchar);
extern __attribute__((visibility("default"))) char* strremcharall(char* str, const char oldchar);
extern __attribute__((visibility("default"))) char* strremcharat(char* str, size_t pos);

extern __attribute__((visibility("default"))) char* strrepsubstrfirst(char* str, const char* oldsubstr, const char* newsubstr);
extern __attribute__((visibility("default"))) char* strrepsubstrall(char* str, const char* oldsubstr, const char* newsubstr);

extern __attribute__((visibility("default"))) char* strrepcharfirst(char* str, const char oldchar, const char newchar);
extern __attribute__((visibility("default"))) char* strrepcharall(char* str, const char oldchar, const char newchar);
extern __attribute__((visibility("default"))) char* strrepcharat(char* str, const char newchar, size_t pos);

extern __attribute__((visibility("default"))) void strsplitkeyvalue(const char* str, const char* delimiter, char **key, char **value);
extern __attribute__((visibility("default"))) char** strsplitsubstr(const char* str, const char* delimiter, size_t *numsubstr);
extern __attribute__((visibility("default"))) char** strsplitchar(const char* str, const char delimiter, size_t *numsubstr);
extern __attribute__((visibility("default"))) char* strjoinwithsubstr(const char **strlist, const char* delimiter);
extern __attribute__((visibility("default"))) char* strjoinwithchar(const char** strlist, const char delimiter);
extern __attribute__((visibility("default"))) void  strfreelist(char** strlist, size_t numsubstr);

#endif
