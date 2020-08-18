#include "MessageBusClient.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

class MessageCallback : public IMessageCallback
{
public:
    MessageCallback()
    {

    }

    virtual ~MessageCallback() override
    {

    }

    void OnNodeOnline(const std::string& nodename) override
    {
        std::cout << nodename << " online" << std::endl;
    }

    void OnNodeOffline(const std::string& nodename) override
    {
        std::cout << nodename << " offline" << std::endl;
    }

    void OnData(const std::string& nodename, const MessageType &mtype, const DataType &dtype, const std::vector<char>& messagebuffer) override
    {

    }

    void OnEvent(const std::string& nodename, const DataType& dtype, const std::vector<char>& messagebuffer) override
    {

    }

    void OnRequest(const std::string& nodename, const DataType& dtype, const long &payloadid, const std::vector<char>& messagebuffer) override
    {

    }

    void OnResponse(const std::string& nodename, const DataType& dtype, const long& payloadid, const std::vector<char>& messagebuffer) override
    {

    }
};

int main(int argc, char* argv[])
{
    MessageCallback callback;

    MessageBusClient client;

    if(!client.Initialize(&callback))
    {
        return -1;
    }

    if(!client.Open())
    {
        return -1;
    }

    if(!client.Register())
    {
        return -1;
    }

    std::cout << "I am Pumba. I am heavy and fat. I use a C++ based IPC\n";

    std::string str = "Hello Timon! I'm Pumba";
    long payload_id = 0;
    int snooze_time = 0;

    client.Send("Pumba", Data, LoopBack, str, payload_id);

    while(snooze_time < 600)
    {
        long node_index = -1;
        node_index = client.HasNode("Timon");

        if(node_index > -1)
        {
            long payload_id = 0;
            std::string node_full_name = client.NodeFullname(node_index);
            client.Send(node_full_name, Data, UserData, str, payload_id);
        }

        std::this_thread::sleep_for (std::chrono::seconds(10));
        snooze_time += 10;
    }

    client.DeRegister();
    client.Close();
    return 0;
}
