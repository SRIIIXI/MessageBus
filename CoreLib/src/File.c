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


#include "File.h"

#define DIRECTORY_SEPARATOR '/'
#include <sys/stat.h>

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#else
#include <dirent.h>
#include <unistd.h>
#endif

bool file_is_exists(const char* filename)
{
	FILE* fp = fopen(filename, "r");

	if(fp)
	{
		fclose(fp);
		return true;
	}

	return false;
}

char* file_get_parent_directory(const char* filename)
{
	size_t origlen = strlen(filename);

	char* parent_dir = (char*)calloc(1, sizeof(char) * (origlen + 1));

	if(parent_dir == NULL)
	{
		return NULL;
	}

	memcpy(parent_dir, filename, origlen);

	int len = (int)strlen(parent_dir);

	if(len < 2)
    {
        free(parent_dir);
        return NULL;
    }

	int ctr = len - 1;

	while(true)
	{
		parent_dir[ctr] = 0;
		ctr--;
		if(parent_dir[ctr] == '/' || parent_dir[ctr] == '\\')
		{
			break;
		}
	}

	return parent_dir;
}

char* file_get_basename(const char* filename)
{
	size_t origlen = strlen(filename);
	size_t index = 0;

	char* basename = (char*)calloc(1, sizeof(char) * (origlen + 1));

	if(basename == NULL)
	{
		return NULL;
	}

	for(index = origlen - 1; filename[index] != '/' && filename[index] != '\\'; index--) { }

	memcpy(basename, &filename[index + 1], index);

	return basename;
}

char* file_get_extension(const char* filename)
{
    size_t origlen = strlen(filename);
    size_t index = 0;

    char* extension = (char*)calloc(1, sizeof(char) * (origlen + 1));

    if(extension == NULL)
    {
        return NULL;
    }

    for(index = origlen - 1; filename[index] != '.'; index--) { }

    memcpy(extension, &filename[index+1], index-1);

    return extension;
}
