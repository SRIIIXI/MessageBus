#ifndef MESSAGE_BROKER_C
#define MESSAGE_BROKER_C

#include <string.h>
#include <stdbool.h>

typedef enum RunState
{
    Running =0,
    NormalExit = 1,
    BindFailed = -1,
    ListenFailed = -2,
    SignalReceived = 3
}RunState;

bool broker_initialize(char* appname, int port);
bool broker_run();
void broker_stop();

/*
class BrokerSignalHandler : public SignalCallback
{
public:
	BrokerSignalHandler();
	void suspend();
	void resume();
	void shutdown();
	void alarm();
	void reset();
	void childExit();
	void userdefined1();
	void userdefined2();
};



class MessageBroker
{
public:
    MessageBroker(std::string appname);
    ~MessageBroker();

	bool initialize(std::string port = "80");
    RunState run();
    void stop();

private:
    int _ListenerSocket;

    int _Port;

	SignalHandler   _SigHdlr;
	BrokerSignalHandler _AppSignals;
};

extern MessageBroker* broker;
*/

#endif
