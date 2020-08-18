#include "Responder.h"
#include "StringEx.h"
#include "StringList.h"

#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET (-1)

#define SOCKET_ERROR	 (-1)
#define LPSOCKADDR sockaddr*

#pragma pack(1)
typedef struct responder
{
    bool			connected;
    SOCKET 			socket;
    struct sockaddr_in		server_address;
    char			server_name[33];
    int				server_port;
    size_t			prefetched_buffer_size;
    unsigned char*	prefetched_buffer;
    int             error_code;
}responder;

bool is_ip4_address(char* str);

bool is_ip4_address(char* str)
{
    size_t slen = strlen(str);

    // Check the string length, for the range ...
    // 0.0.0.0 and 255.255.255.255
    if(slen < 7 || slen > 15)
    {
        // Bail out
        return false;
    }

    int ctr;
    bool isdelimeter = false;
    char nibble[4];
    memset((char*)&nibble[0],0,4);
    int nbindex = 0;
    for(ctr = 0 ; str[ctr] != '\0' ; ctr++)
    {
        // Check for permitted characters
        if(str[ctr] != '.' && isdigit(str[ctr]) <= 0)
        {
            // Bail out
            return false;
        }

        // '.' Delimeter case
        if(str[ctr] == '.')
        {
            if(isdelimeter)
            {
                // The flag was set in last iteration
                // This means ".." type of expression was found
                // Bail out
                return false;
            }

            // We have read a complete nibble
            // The characters in the nibble must represent a permissible value
            int numval = atoi(nibble);
            if(numval < 0 || numval > 255)
            {
                return false;
            }

            // Set the flag and continue
            memset((char*)&nibble[0],0,4);
            nbindex = 0;
            isdelimeter = true;
            continue;
        }

        if(isdigit(str[ctr])> 0)
        {
            isdelimeter = false;
            nibble[nbindex] = str[ctr];
            nbindex++;
            continue;
        }

    }

    return true;
}


void* responder_create_socket(void* ptr, const char* servername, int serverport)
{
    struct responder* responder_ptr =   (struct responder*)calloc(1, sizeof (struct responder));

    ptr = responder_ptr;

    if(!responder_ptr)
    {
        return  NULL;
    }

    strncpy(responder_ptr->server_name, servername, 32);
    responder_ptr->server_port = serverport;

    responder_ptr->server_address.sin_family = AF_INET;
    responder_ptr->server_address.sin_port = htons(serverport);

    u_long nRemoteAddr;

    char ipbuffer[32]={0};
    strncpy(ipbuffer, servername, 31);

    bool ip = is_ip4_address(ipbuffer);

    if(!ip)
    {
        struct hostent* pHE = gethostbyname(responder_ptr->server_name);
        if (pHE == 0)
        {
            nRemoteAddr = INADDR_NONE;
            free(ptr);
            return NULL;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
        responder_ptr->server_address.sin_addr.s_addr = nRemoteAddr;
    }
    else
    {
         inet_pton (AF_INET, responder_ptr->server_name, &responder_ptr->server_address.sin_addr);
    }

    responder_ptr->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(responder_ptr->socket == INVALID_SOCKET)
    {
        free(ptr);
        return NULL;
    }

    return ptr;
}

void* responder_assign_socket(void* ptr, int inSocket)
{
    struct responder* responder_ptr =   (struct responder*)calloc(1, sizeof (struct responder));

    ptr = responder_ptr;

    if(!responder_ptr)
    {
        return  NULL;
    }

    responder_ptr->socket = inSocket;
    responder_ptr->connected = true;
    return ptr;
}

bool responder_connect_socket(void* ptr)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    if(responder_ptr->connected == true)
	{
		return true;
	}

    int returncode = -1;

    returncode = connect(responder_ptr->socket,(struct sockaddr*)&responder_ptr->server_address, sizeof(struct sockaddr_in));

    if(returncode == SOCKET_ERROR)
	{
        responder_ptr->error_code = errno;
        shutdown(responder_ptr->socket, 2);
        close(responder_ptr->socket);
        responder_ptr->connected = false;
		return false;
	}

    responder_ptr->connected = true;
	return true;
}

bool responder_close_socket(void* ptr)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    if(responder_ptr->connected == false)
    {
        return true;
    }

    shutdown(responder_ptr->socket, 2);
    close(responder_ptr->socket);

    responder_ptr->connected = false;
    free(responder_ptr);
	return false;
}

bool responder_receive_buffer(void* ptr, char** iobuffer, size_t len, bool alloc_buffer)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    size_t	bufferpos = 0;
    size_t	bytesleft = len;

    // If there are pre-fetched bytes left, we have to copy that first and release memory

    if(alloc_buffer)
    {
        *iobuffer = (char*)calloc(1, len + 1);
    }

    if(responder_ptr->prefetched_buffer_size > 0)
    {
        memcpy(*iobuffer, responder_ptr->prefetched_buffer, responder_ptr->prefetched_buffer_size);
        bytesleft = len - responder_ptr->prefetched_buffer_size;
        bufferpos = responder_ptr->prefetched_buffer_size;
        responder_ptr->prefetched_buffer_size = 0;
        free(responder_ptr->prefetched_buffer);
        responder_ptr->prefetched_buffer = NULL;

        if(bytesleft < 1)
        {
            return true;
        }
    }

    while(true)
    {
        char*	buffer = 0;
        ssize_t	bytesread = 0;
        buffer = (char*)calloc(1, bytesleft + 1);

        if (buffer)
        {
            bytesread = (ssize_t)recv(responder_ptr->socket, buffer, (int)bytesleft, 0);
        }

        // Error or link down
        if(bytesread < 1 || buffer == NULL)
        {
            responder_ptr->error_code = SOCKET_ERROR;

            if (buffer)
            {
                free(buffer);
            }

            if(alloc_buffer)
            {
                free(*iobuffer);
            }
            else
            {
                memset(*iobuffer, 0, len);
            }

            len	= 0;
            responder_ptr->connected = false;
            return false;
        }

        memcpy(*iobuffer+bufferpos, buffer, (size_t)bytesread);
        free(buffer);

        bufferpos = bufferpos + (size_t)bytesread;

        bytesleft = bytesleft - (size_t)bytesread;

        if(bufferpos >= len)
        {
            return true;
        }
    }
}

bool responder_receive_string(void* ptr, char** iostr, const char* delimeter)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    char*	data = NULL;
    char*   current_line = NULL;
    char*   next_line = NULL;

    if(responder_ptr->prefetched_buffer_size > 0)
	{
        if(strstr((char*)responder_ptr->prefetched_buffer, delimeter) !=0 )
		{
            strsplitkeyvalue((const char*)responder_ptr->prefetched_buffer, delimeter, &current_line, &next_line);

            responder_ptr->prefetched_buffer = NULL;
            free(responder_ptr->prefetched_buffer);
            responder_ptr->prefetched_buffer_size = 0;

            if(current_line != NULL)
            {
                *iostr = (char*)calloc(1, strlen(current_line));
                strcpy(*iostr, current_line);
                free(current_line);
            }

            if(next_line != NULL)
            {
                responder_ptr->prefetched_buffer_size = strlen(next_line);
                responder_ptr->prefetched_buffer = (unsigned char*)calloc(1, (sizeof (unsigned char)*responder_ptr->prefetched_buffer_size) + 1);
                strcpy((char*)responder_ptr->prefetched_buffer, next_line);
                free(next_line);
            }

			return true;
		}

        if(responder_ptr->prefetched_buffer_size > 0)
        {
            data = (char*)calloc(1, responder_ptr->prefetched_buffer_size + 1);
            strcpy(data, (char*)responder_ptr->prefetched_buffer);
            responder_ptr->prefetched_buffer_size = 0;
            free(responder_ptr->prefetched_buffer);
            responder_ptr->prefetched_buffer = NULL;
        }
	}

	while(true)
	{
        char* buffer = NULL;

        if(!responder_receive_buffer(ptr, &buffer, 1024, true))
        {
            if(*iostr)
            {
                free(*iostr);
            }

            responder_ptr->connected = false;
            responder_ptr->error_code = SOCKET_ERROR;
            return false;
        }

        data = (char*)realloc(data, strlen(data) + 1024);
        strcat(data, buffer);
        free(buffer);

        if(strstr(data, delimeter) != 0)
		{
            strsplitkeyvalue((const char*)responder_ptr->prefetched_buffer, delimeter, &current_line, &next_line);

            if(next_line != NULL)
            {
                responder_ptr->prefetched_buffer_size = strlen(next_line);
            }
            
            if(responder_ptr->prefetched_buffer_size > 0)
            {
                responder_ptr->prefetched_buffer = (unsigned char*)calloc(1, sizeof (unsigned char));
                memcpy(responder_ptr->prefetched_buffer, next_line, responder_ptr->prefetched_buffer_size);
                free(next_line);
            }

            *iostr = (char*)realloc(*iostr, strlen(*iostr) + strlen(current_line));
            strcat(*iostr, current_line);
            free(current_line);
            free(data);

            return true;
		}
	}
	return true;
}

bool responder_send_buffer(void* ptr, const char* data, size_t len)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

	long sentsize =0;

    sentsize = send(responder_ptr->socket, data, len, 0);

    if(sentsize == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool responder_send_string(void* ptr, const char* str)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    size_t len = strlen(str);

    long sentsize =0;

    sentsize = send(responder_ptr->socket, str, len, 0);

    if(sentsize == SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

size_t responder_read_size(void* ptr)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    return responder_ptr->prefetched_buffer_size;
}

bool responder_is_connected(void* ptr)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    return responder_ptr->connected;
}

int responder_get_socket(void* ptr)
{
    struct responder* responder_ptr = (struct responder*)ptr;

    if(!responder_ptr)
    {
        return  false;
    }

    if(responder_ptr->connected)
    {
        return responder_ptr->socket;
    }

    return -1;
}
