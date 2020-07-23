#ifndef PAYLOAD_C
#define PAYLOAD_C

#define PAYLOAD_TYPE_EVENT 'E'
#define PAYLOAD_TYPE_DATA 'D'
#define PAYLOAD_TYPE_REQUEST 'Q'
#define PAYLOAD_TYPE_RESPONSE 'R'

#define PAYLOAD_SUB_TYPE_REQUEST_CONFIGURATION 'C'
#define PAYLOAD_SUB_TYPE_REQUEST_POLICY 'P'

#define PAYLOAD_SUB_TYPE_RESPONSE_CONFIGURATION 'c'
#define PAYLOAD_SUB_TYPE_RESPONSE_POLICY 'p'

#define PAYLOAD_SUB_TYPE_REGISTER 'G'
#define PAYLOAD_SUB_TYPE_DEREGISTER 'D'

#define PAYLOAD_SUB_TYPE_NODE_ONLINE 'N'
#define PAYLOAD_SUB_TYPE_NODE_OFFLINE 'F'

#define PAYLOAD_SUB_TYPE_NODELIST 'L'
#define PAYLOAD_SUB_TYPE_LOOPBACK 'K'

#pragma push(1)
typedef struct payload
{
    char payload_type;
    char payload_sub_type;
    char sender[32];
    char receipient[32];
    long data_size;
    void* data;
}payload;

#endif
