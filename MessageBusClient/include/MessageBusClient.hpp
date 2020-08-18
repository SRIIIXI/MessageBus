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

static IMessageCallback* callback = nullptr;

class MessageBusClient
{
private:
    void* message_bus;

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
        return true;
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
        return message_bus_send(message_bus, node_name.c_str(), ptype, mtype, dtype, messagebuffer, buffersize, &payload_id);
	}

	bool HasNode(const std::string &node_name)
	{
		return message_bus_has_node(message_bus, node_name.c_str());
	}

	static void NetworkEvent(const char* node_name, PayloadType ptype, MessageType mtype, DataType dtype, const char* messagebuffer, long buffersize, long payload_id)
	{
		if (callback)
		{
			switch (ptype)
			{
			Event:
				{
					if (mtype == NodeOnline)
					{
						callback->OnNodeOnline(std::string(node_name));
						break;
					}

					if (mtype == NodeOffline)
					{
						callback->OnNodeOffline(std::string(node_name));
						break;
					}

                    std::vector<char> buffer = messagebuffer;

					callback->OnEvent(std::string(node_name), dtype, buffer);

					break;
				}
			Request:
				{
					std::vector<char> buffer = messagebuffer;

					callback->OnRequest(std::string(node_name), dtype, payload_id, buffer);

					break;
				}			
			Response:
				{
					std::vector<char> buffer = messagebuffer;

					callback->OnResponse(std::string(node_name), dtype, payload_id, buffer);

					break;
				}			
			Data:
				{
					std::vector<char> buffer = messagebuffer;

					callback->OnData(std::string(node_name), mtype, dtype, payload_id, buffer);

					break;
				}				
			default:
				break;
			}
		}
	}
};
