#include "MessageBusClient.h"
#include <string>
#include <list>
#include <vector>

class IMessageCallback
{
public:
	IMessageCallback() {}
	virtual ~IMessageCallback() {}
	virtual void OnNodeOnline(const std::string& nodename) = 0;
	virtual void OnNodeOffline(const std::string& nodename) = 0;
	virtual void OnData(const std::string& nodename, const MessageType &mtype, const DataType &dtype, const std::vector<char>& messagebuffer) = 0;
	virtual void OnEvent(const std::string& nodename, const DataType& dtype, const std::vector<char>& messagebuffer) = 0;
	virtual void OnRequest(const std::string& nodename, const DataType& dtype, const long &payloadid, const std::vector<char>& messagebuffer) = 0;
	virtual void OnResponse(const std::string& nodename, const DataType& dtype, const long& payloadid, const std::vector<char>& messagebuffer) = 0;
};

class MessageBusClient
{
private:
    void* message_bus;

public:

    MessageBusClient();
    ~MessageBusClient();
    bool Initialize(IMessageCallback *callbackref);
    bool Open();
    bool Close();
    bool Register();
    bool DeRegister();
    bool Send(const std::string &node_name, PayloadType ptype, MessageType mtype, const std::string messagebuffer, long &payload_id);
    bool Send(const std::string &node_name, PayloadType ptype, MessageType mtype, DataType dtype, const std::vector<char> messagebuffer, long &payload_id);
    bool HasNode(const std::string &node_name);
    std::string NodeFullname(long index);
};
