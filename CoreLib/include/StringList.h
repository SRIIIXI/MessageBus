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

#ifndef STRING_LIST_C
#define STRING_LIST_C

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32) || defined (WIN32) || defined (_WIN64)
#define LIBRARY_EXPORT __declspec(dllexport)
#define LIBRARY_ENTRY
#define LIBRARY_EXIT 
#else
#define LIBRARY_EXPORT __attribute__((visibility("default")))
#define LIBRARY_ENTRY __attribute__((constructor))
#define LIBRARY_EXIT __attribute__((destructor))
#endif 

extern LIBRARY_EXPORT void* str_list_allocate(void* lptr);
extern LIBRARY_EXPORT void* str_list_allocate_from_string(void* lptr, const char* str, const char* delimiter);
extern LIBRARY_EXPORT void str_list_clear(void* lptr);

extern LIBRARY_EXPORT void str_list_add_to_head(void* lptr, char* data);
extern LIBRARY_EXPORT void str_list_add_to_tail(void* lptr, char* data);
extern LIBRARY_EXPORT void str_list_insert(void* lptr, char* data, size_t pos);

extern LIBRARY_EXPORT void str_list_remove_from_head(void* lptr);
extern LIBRARY_EXPORT void str_list_remove_from_tail(void* lptr);
extern LIBRARY_EXPORT void str_list_remove(void* lptr, const char* node);
extern LIBRARY_EXPORT void str_list_remove_at(void* lptr, size_t pos);
extern LIBRARY_EXPORT void str_list_remove_value(void* lptr, char* data);

extern LIBRARY_EXPORT size_t str_list_item_count(void* lptr);
extern LIBRARY_EXPORT long long str_list_index_of(void* lptr, char* data);
extern LIBRARY_EXPORT char* str_list_get_at(void* lptr, size_t atpos);

extern LIBRARY_EXPORT char* str_list_get_first(void* lptr);
extern LIBRARY_EXPORT char* str_list_get_next(void* lptr);
extern LIBRARY_EXPORT char* str_list_get_last(void* lptr);
extern LIBRARY_EXPORT char* str_list_get_previous(void* lptr);

extern LIBRARY_EXPORT void str_list_sort(void* lptr);
extern LIBRARY_EXPORT void str_list_merge(void* lptrFirst, void* lptrSecond);
extern LIBRARY_EXPORT void str_list_join(void* lptrFirst, void* lptrSecond);


#endif
