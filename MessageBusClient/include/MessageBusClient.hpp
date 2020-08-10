#include "MessageBusClient.h"
#include <string>
#include <list>

class IMessageCallback
{

};

class MessageBusClient : public IMessageCallback
{
public:
	MessageBusClient()
	{

	}

	virtual ~MessageBusClient()
	{

	}

	bool Initialize(IMessageCallback *callbackref)
	{
		callback = callbackref;
	}

	bool Open()
	{
		return message_bus_initialize(&message_bus, NetworkEvent);
	}

	bool Close()
	{
		return message_bus_close(message_bus);
	}

	bool Register()
	{
		return message_bus_register(message_bus);
	}

	bool DeRegister()
	{
		return message_bus_deregister(message_bus);
	}

	bool Send(const std::string &node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long &payload_id)
	{
		return message_bus_send(message_bus, node_name.c_str(), ptype, mtype.dtype, messagebuffer, buffersize, *payload_id);
	}

	bool HasNode(const std::string &node_name)
	{
		return message_bus_has_node(message_bus, node_name.c_str());
	}

	static void NetworkEvent(const std::string& node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long payload_id)
	{

	}

private:
	void* message_bus;
	IMessageCallback* callback;
};