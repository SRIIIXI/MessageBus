#include "MessageBroker.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int listening_port = 49151;

    if(argc > 1)
    {
        listening_port = atoi(argv[1]);
    }

    if(!broker_initialize(argv[0], listening_port))
    {
        return -1;
    }

    broker_run();

	return 0;
}
