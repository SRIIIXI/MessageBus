#ifndef	RESPONDER_C
#define	RESPONDER_C

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern __attribute__((visibility("default"))) void* responder_create_socket(void* ptr, const char* servername, int serverport);
extern __attribute__((visibility("default"))) void* responder_assign_socket(void* ptr, int inSocket);
extern __attribute__((visibility("default"))) bool responder_connect_socket(void* ptr);
extern __attribute__((visibility("default"))) bool responder_close_socket(void* ptr);
extern __attribute__((visibility("default"))) bool responder_send_buffer(void* ptr, const char* data, size_t len);
extern __attribute__((visibility("default"))) bool responder_send_string(void* ptr, const char* str);
extern __attribute__((visibility("default"))) bool responder_receive_buffer(void* ptr, char** iobuffer, size_t len, bool alloc_buffer);
extern __attribute__((visibility("default"))) bool responder_receive_string(void* ptr, char** iostr, const char* delimeter);
extern __attribute__((visibility("default"))) size_t  responder_read_size(void* ptr);
extern __attribute__((visibility("default"))) bool responder_is_connected(void* ptr);
extern __attribute__((visibility("default"))) int  responder_get_socket(void* ptr);
extern __attribute__((visibility("default"))) int  responder_get_error_code(void* ptr);

#endif

