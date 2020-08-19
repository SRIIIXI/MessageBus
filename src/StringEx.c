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

#include "StringEx.h"
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

wchar_t *strtowstr(const char *str)
{
    return NULL;
}

char* strfromwstr(const wchar_t* wstr)
{
	if(wstr == NULL)
	{
		return NULL;
	}

    size_t wlen = 0;
    char* str = NULL;

    for (wlen = 0; wstr[wlen] != '\0'; wlen++) {}

    str = (char*)calloc(1, wlen+1);

    if (str != NULL)
    {
        for (size_t idx = 0; idx < wlen; idx++)
        {
            str[idx] = (char)wstr[idx];
        }
    }

    return str;
}

char* strfromint(size_t num)
{
	char* ptr = (char*)calloc(1, (size_t)32);

	if (ptr == NULL)
	{
		return NULL;
	}

	int sign = 1;
    size_t remainder = 1;
    size_t dividend = num;
	size_t ctr = 0;

	if (num < 1)
	{
		sign = -1;
        dividend = dividend*(size_t)(-1);
	}

    while (dividend && ctr < 32)
	{
		remainder = dividend % 10;
		dividend = dividend / 10;

        ptr[ctr] = (size_t)(remainder + 48);
		ctr++;
	}

	if (sign < 1)
	{
		ptr[ctr] = '-';
	}
	else
	{
		ctr--;
	}

	size_t start = 0;

	while (start < ctr)
	{
		char temp = ptr[start];
		ptr[start] = ptr[ctr];
		ptr[ctr] = temp;
		start++;
		ctr--;
	}

	return ptr;
}

char* strfromdouble(double num)
{
	return NULL;
}

char* strreverse(char* ptr)
{
	size_t start = 0;

	size_t term = strlen(ptr) - 1;

	while (start < term)
	{
		char temp = ptr[start];
		ptr[start] = ptr[term];
		ptr[term] = temp;
		start++;
		term--;
	}

	return ptr;
}

char* strsegmentreverse(char* str, size_t start, size_t term)
{
	while (start < term)
	{
        char temp = str[start];
        str[start] = str[term];
        str[term] = temp;
		start++;
		term--;
	}

    return str;
}

long long strindexofsubstr(char* str, const char* substr)
{
    long long result = -1;

    char* pdest = (char*)strstr( str, substr );

    if(pdest == 0)
    {
        return -1;
    }

    result = pdest - str;

    return result;
}

long long strindexofchar(char* str, const char ch)
{
    for (int ctr = 0; str[ctr] != '\0'; ctr++)
    {
        if (str[ctr] == ch)
        {
            return ctr;
        }
    }

    return -1;
}

size_t strcountsubstr(char* str, const char* substr)
{
	size_t ctr = 0;

	size_t offset = strlen(substr);

	char* ptr = str;

	bool contiue_scan = true;

	while (contiue_scan)
	{
		long long index = strindexofsubstr(ptr, substr);

		if (index > -1)
		{
			ptr = ptr + index + offset;
			ctr++;
			contiue_scan = true;
		}
		else
		{
			contiue_scan = false;
		}
	}

    return ctr;
}

size_t strcountchar(char* str, const char ch)
{
	size_t ctr = 0;

	for (int index = 0; str[index] != '\0'; index++)
	{
		if (str[index] == ch)
		{
			ctr++;
		}
	}

	return ctr;
}

extern char* strtolower(char* str)
{
    for (size_t ctr = 0; str[ctr] != '\0'; ctr++)
    {
        if (str[ctr] >= 65 && str[ctr] <= 90)
        {
            str[ctr] = str[ctr] + 32;
        }
    }

    return str;
}

extern char* strtoupper(char* str)
{
    for (size_t ctr = 0; str[ctr] != '\0'; ctr++)
    {
        if (str[ctr] >= 97 && str[ctr] <= 122)
        {
            str[ctr] = str[ctr] - 32;
        }
    }

    return str;
}

char* strlefttrim(char* str)
{
    char *ptr = str;

    int ctr = 0;

    while (isspace(*ptr))
    {
        ptr++;
    }

    while (*ptr)
    {
        str[ctr] = *ptr;
        ctr++;
        ptr++;
    }

	while (str[ctr] != '\0')
	{
		str[ctr] = '\0';
		ctr++;
	}

    return str;
}

char* strrighttrim(char* str)
{
    size_t len = strlen(str);

    for (int ctr = len - 1; ctr > -1; ctr--)
    {
        if (isspace(str[ctr]))
        {
            str[ctr] = '\0';
        }
        else
        {
            break;
        }
    }

    return str;
}

char* stralltrim(char* str)
{
    strrighttrim(str);
    strlefttrim(str);
    return str;
}

char* strremsubstrfirst(char* str, const char* substr)
{
    int pos = -1;
    int offset = strlen(substr);

    pos = strindexofsubstr(str, substr);

    if(pos >= 0)
    {
        strcpy(str+pos, str+pos+offset);
        str[strlen(str) - offset] = 0;
    }
    return str;
}

char* strremsubstrall(char* str, const char* substr)
{
    int pos = -1;
    int offset = strlen(substr);

    pos = strindexofsubstr(str, substr);

    while(pos >= 0)
    {
        strcpy(str+pos, str+pos+offset);
        str[strlen(str) - offset] = 0;
        pos = strindexofsubstr(str, substr);
    }
    return str;
}

char* strremsubstrat(char* str, size_t pos, size_t len)
{
    if(pos >= 0 && pos <= (strlen(str)-1) )
    {
        strcpy(str+pos, str+pos+len);
        str[strlen(str) - len] = 0;
    }
    return str;
}

char* strremcharfirst(char* str, const char oldchar)
{
    int pos = strindexofchar(str, oldchar);
    strcpy(str+pos, str+pos+1);
    str[strlen(str) - 1] = 0;
    return str;
}

char* strremcharall(char* str, const char oldchar)
{
    int pos = strindexofchar(str, oldchar);

    while(pos >= 0)
    {
        strcpy(str+pos, str+pos+1);
        str[strlen(str) - 1] = 0;
        pos = strindexofchar(str, oldchar);
    }
    return str;
}

char* strremcharat(char* str, size_t pos)
{
    strcpy(str+pos, str+pos+1);
    str[strlen(str) - 1] = 0;
    return str;
}

char* strrepsubstrfirst(char* str, const char* oldsubstr, const char* newsubstr)
{
	if(str == NULL || oldsubstr == NULL || newsubstr == NULL)
	{
		return NULL;
	}

	char* buffer = NULL;

	long long pos = strindexofsubstr(str, oldsubstr);

	if(pos < 0)
	{
		return NULL;
	}

	size_t slen = strlen(str);
	size_t oldslen = strlen(oldsubstr);
	size_t newslen = strlen(newsubstr);

	if(oldslen < 1 || newslen < 1)
	{
		return NULL;
	}

	if(newslen > oldslen)
	{
		buffer = (char*)calloc(slen + (newslen - oldslen) + 1, sizeof(char));

		if(buffer == NULL)
		{
			return NULL;
		}
	}
	else
	{
		buffer = str;
	}

	size_t idx = 0;
	size_t ctr = 0;

	for(idx = 0; idx < slen; ++idx)
	{
		if(idx < pos)
		{
			buffer[idx] = str[idx];
		}
		else
		{
			if(idx < pos + newslen)
			{
				buffer[idx] = newsubstr[ctr];
				ctr++;
			}
			else
			{
				buffer[idx] = buffer[idx + (oldslen - newslen)];
			}
		}
	}
		
    return buffer;
}

char* strrepsubstrall(char* str, const char* oldsubstr, const char* newsubstr)
{
	char* buffer = NULL;

	if(str == NULL || oldsubstr == NULL || newsubstr == NULL)
	{
		return NULL;
	}

	size_t slen = strlen(str);
	size_t oldslen = strlen(oldsubstr);
	size_t newslen = strlen(newsubstr);

	if(oldslen < 1 || newslen < 1)
	{
		return NULL;
	}

	size_t numsubstr = strcountsubstr(str, oldsubstr);

	if(numsubstr < 1)
	{
		return NULL;
	}

	if(newslen > oldslen)
	{
		buffer = (char*)calloc(slen + (newslen - oldslen)*numsubstr + 1, sizeof(char));

		if(buffer == NULL)
		{
			return NULL;
		}
	}
	else
	{
		buffer = str;
	}

	long long pos = strindexofsubstr(str, oldsubstr);

	while(pos > -1)
	{
		size_t idx = 0;
		size_t ctr = 0;

		for(idx = 0; idx < slen; ++idx)
		{
			if(idx < pos)
			{
				buffer[idx] = str[idx];
			}
			else
			{
				if(idx < pos + newslen)
				{
					buffer[idx] = newsubstr[ctr];
					ctr++;
				}
				else
				{
					buffer[idx] = buffer[idx + (oldslen - newslen)];
				}
			}
		}

		pos = strindexofsubstr(str, oldsubstr);
	}

	return buffer;
}

char* strrepcharfirst(char* str, const char oldchar, const char newchar)
{
	if(str != NULL)
	{
		for(size_t pos = 0; str[pos] != 0; pos++)
		{
			if(str[pos] == oldchar)
			{
				str[pos] = newchar;
				return str;
			}
		}
		return str;
	}
	return NULL;
}

char* strrepcharall(char* str, const char oldchar, const char newchar)
{
    if(str != NULL)
    {
        for(size_t pos = 0; str[pos] != 0; pos++)
        {
            if(str[pos] == oldchar)
            {
                str[pos] = newchar;
            }
        }
        return str;
    }
    return NULL;
}

char* strrepcharat(char* str, const char newchar, size_t pos)
{
    if(str != NULL)
    {
        if(pos < strlen(str))
        {
            str[pos] = newchar;
            return str;
        }
    }

    return NULL;
}

void strsplitkeyvalue(const char* str, const char* delimiter, char **key, char **value)
{
    if(str == NULL || delimiter == NULL)
    {
        return;
    }

    long pos = (size_t)strindexofsubstr(str, delimiter);

    if(pos < 0)
    {
        return;
    }

    size_t val_start = ((size_t)pos + strlen(delimiter));
    size_t val_end = strlen(str);

    if(pos > 0)
    {
        *key = (char*)calloc(1, (size_t)(pos + 1));
        strcpy(*key, &str[pos]);
    }

    *value = (char*)calloc(1, val_end - val_start + 1);
    strcpy(*value, &str[val_start]);

}

extern char** strsplitsubstr(const char* str, const char* delimiter, size_t *numsubstr)
{
	if(str == NULL || delimiter == NULL)
	{
		return NULL;
	}

	size_t substr_count = strcountsubstr(str, delimiter);
	size_t str_len = strlen(str);
	size_t index = 0;

	*numsubstr = substr_count;

	if(substr_count < 1)
	{
		return NULL;
	}

	char* ptr = (char*)calloc(1, str_len);

	if(ptr == NULL)
	{
		return NULL;
	}

	memcpy(ptr, str, str_len);

	char** buffer = NULL;

	buffer = (char*)calloc(1, sizeof(char) * substr_count + 1);

	if(buffer == NULL)
	{
        free(ptr);
		return NULL;
	}

	char* temp_ptr = NULL;

	temp_ptr = strtok(ptr, delimiter);

	while(temp_ptr != NULL)
	{
		size_t temp_str_len = strlen(temp_ptr);

		buffer[index] = (char*)calloc(1, sizeof(char) * (temp_str_len + 1));

		if(buffer[index] == NULL)
		{
			return NULL;
		}

		memcpy(buffer[index], temp_ptr, temp_str_len);

		temp_ptr = strtok(NULL, delimiter);
		index++;
	}

	return buffer;
}

extern char** strsplitchar(const char* str, const char delimiter, size_t *numsubstr)
{
	char temp_delimiter[2] = {delimiter, 0};

	return strsplitsubstr(str, temp_delimiter, numsubstr);
}

extern char* strjoinwithsubstr(const char** strlist, const char* delimiter)
{
	return NULL;
}

extern char* strjoinwithchar(const char** strlist, const char delimiter)
{
	char temp_delimiter[2] = { delimiter, 0 };

	return strjoinwithsubstr(strlist, temp_delimiter);
}

void strfreelist(char** strlist, size_t numsubstr)
{
	size_t index = numsubstr;
	
	while(index > 0)
	{
		if(strlist[index - 1] != NULL)
		{
			char* ptr =  strlist[index - 1];
			free(strlist[index - 1]);
		}

		index--;
	}

	free(strlist);
}
