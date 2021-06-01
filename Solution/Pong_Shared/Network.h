#pragma once

#include <winsock2.h>
#include "NetworkMessage.h"
#include "Mutex.h"
#include "..\Framework\FW_Assert.h"
#include <map>
#include "..\Framework\FW_TypeID.h"

#pragma comment(lib, "Ws2_32.lib")

#define THREAD_MODE 1

class INetwork_MessageBuss
{
public:
	virtual void SendMessage(NetworkSerializationStreamType& someData, const sockaddr_in& aSenderAddress) = 0;
};

template <typename MessageType>
class Network_MessageBuss : public INetwork_MessageBuss
{
public:
	typedef std::function<void(const MessageType&, const sockaddr_in&)> CallbackFunc;

	int Subscribe(CallbackFunc aFunction);
	void Unsubscribe(int aID);

	void SendMessage(NetworkSerializationStreamType& someData, const sockaddr_in& aSenderAddress)
	{
		MessageType message;
		message.DeserializeMessage(someData);

		for (std::pair<int, CallbackFunc> element : myCallbacks)
			element.second(message, aSenderAddress);
	}

private:
	std::map<int, CallbackFunc> myCallbacks;
	int myNextFreeID = 0;
};

template <typename MessageType>
int Network_MessageBuss<MessageType>::Subscribe(CallbackFunc aFunction)
{
	myCallbacks[myNextFreeID] = aFunction;
	return myNextFreeID++;
}

template <typename MessageType>
void Network_MessageBuss<MessageType>::Unsubscribe(int aID)
{
	myCallbacks.erase(aID);
}


namespace std
{
	class thread;
}

class Network
{
public:
	Network();
	~Network();

	void Start();
	void ProcessMessages();

	template <typename MessageType>
	void RegisterMessageType();

	template <typename MessageType>
	int SubscribeToMessage(std::function<void(const MessageType&, const sockaddr_in&)>  aFunction);

	void SendPackedNetworkMessage(const NetworkSerializationStreamType& someData, const sockaddr_in& aTargetAddress);

protected:
	virtual void OnStart() = 0;

	SOCKET mySocket;

	// Data that will be consumed by the Send-Thread and sent over the network
	std::vector<NetworkData> myActiveOutgoingNetworkData;
	// Data that is getting filled by the main program, will be synced over to the 'Active'-list at the start of each SendThread-execution
	std::vector<NetworkData> myOutgoingNetworkDataBuffer;

	// Will be fed with data that is getting read from the network in the RecieveThread, gets copied over to the 'Buffer'-list at the end of each RecieveThread-execution
	std::vector<NetworkData> myActiveIncommingNetworkData;
	// Contains all the latest incomming networkdata to be consumed by the main program
	std::vector<NetworkData> myIncommingNetworkDataBuffer;

private:
#if THREAD_MODE
	void SendAndReceiveThreadFunction();

	volatile bool myIsRunning;
	std::thread* mySendAndReceiveThread;
	Mutex mySendAndReceiveMutex;

#endif

	std::vector<NetworkData> myNetworkData;

	template <typename MessageType>
	Network_MessageBuss<MessageType>& GetMessageBus();

	void SendMessage(int aMessageType, NetworkSerializationStreamType& someData, const sockaddr_in& aSenderAddress)
	{
		INetwork_MessageBuss* bus = myMessageBusses[aMessageType];
		FW_ASSERT(bus != nullptr, "Unregistered MessageType!");

		bus->SendMessage(someData, aSenderAddress);
	}

	static const int myMaxNumMessageBusses = 32;
	INetwork_MessageBuss* myMessageBusses[myMaxNumMessageBusses];
};

template <typename MessageType>
void Network::RegisterMessageType()
{
	int messageTypeID = GetNetworkMessageID<MessageType>();
	FW_ASSERT(messageTypeID < myMaxNumMessageBusses, "Too many MessageTypes, figure out smarter MessageQueue-Storage");
	FW_ASSERT(myMessageBusses[messageTypeID] == nullptr, "MessageType already registered");

	myMessageBusses[messageTypeID] = new Network_MessageBuss<MessageType>();
}

template <typename MessageType>
int Network::SubscribeToMessage(std::function<void(const MessageType&, const sockaddr_in&)>  aFunction)
{
	Network_MessageBuss<MessageType>& bus = GetMessageBus<MessageType>();
	return bus.Subscribe(aFunction);
}

template <typename MessageType>
Network_MessageBuss<MessageType>& Network::GetMessageBus()
{
	int messageTypeID = GetNetworkMessageID<MessageType>();
	INetwork_MessageBuss* bus = myMessageBusses[messageTypeID];
	FW_ASSERT(bus != nullptr, "Unregistered MessageType!");

	return *static_cast<Network_MessageBuss<MessageType>*>(bus);
}