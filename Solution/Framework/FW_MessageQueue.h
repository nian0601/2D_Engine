#pragma once

#include "FW_TypeID.h"

#include <functional>
#include <map>
#include <vector>

class EntityManager;

struct FW_IMessageBus
{
	virtual void SendQueuedMessages() = 0;
};

template <typename MessageType>
class FW_MessageBus : public FW_IMessageBus
{
public:
	typedef std::function<void(const MessageType&)> CallbackFunc;

	int Subscribe(CallbackFunc aFunction);
	void Unsubscribe(int aID);

	void QueueMessage(const MessageType& aMessage);

	void SendQueuedMessages() override
	{
		for (const MessageType& message : myMessageQueue)
		{
			for (std::pair<int, CallbackFunc> element : myCallbacks)
				element.second(message);
		}

		myMessageQueue.clear();
	}

	void InstantlySendMessage(const MessageType& aMessage)
	{
		for (std::pair<int, CallbackFunc> element : myCallbacks)
			element.second(aMessage);
	}

private:
	std::map<int, CallbackFunc> myCallbacks;
	std::vector<MessageType> myMessageQueue;
	int myNextFreeID = 0;
};

template <typename MessageType>
int FW_MessageBus<MessageType>::Subscribe(CallbackFunc aFunction)
{
	myCallbacks[myNextFreeID] = aFunction;
	return myNextFreeID++;
}

template <typename MessageType>
void FW_MessageBus<MessageType>::Unsubscribe(int aID)
{
	myCallbacks.erase(aID);
}

template <typename MessageType>
void FW_MessageBus<MessageType>::QueueMessage(const MessageType& aMessage)
{
	myMessageQueue.push_back(aMessage);
}

//////////////////////////////////////////////////////////////////////////

class FW_MessageQueue
{
public:
	template <typename MessageType>
	void RegisterMessageType();

	template <typename MessageType>
	int SubscribeToMessage(std::function<void(const MessageType&)> aFunction);

	template <typename MessageType>
	void QueueMessage(const MessageType& aMessageType);

	void SendQueuedMessages()
	{
		for (int i = 0; i < myMaxNumMessageBusses; ++i)
		{
			if (myMessageBusses[i])
				myMessageBusses[i]->SendQueuedMessages();
		}
	}

	template <typename MessageType>
	void InstantlySendMessage(const MessageType& aMessage);

private:
	struct BaseMessage {};

	template <typename MessageType>
	FW_MessageBus<MessageType>& GetMessageBus();

	static const int myMaxNumMessageBusses = 32;
	FW_IMessageBus* myMessageBusses[myMaxNumMessageBusses];
};

template <typename MessageType>
void FW_MessageQueue::RegisterMessageType()
{
	int messageTypeID = FW_TypeID<BaseMessage>::GetID<MessageType>();
	FW_ASSERT(messageTypeID < myMaxNumMessageBusses, "Too many MessageTypes, figure out smarter MessageQueue-Storage");
	FW_ASSERT(myMessageBusses[messageTypeID] == nullptr, "MessageType already registered");

	myMessageBusses[messageTypeID] = new FW_MessageBus<MessageType>();
}

template <typename MessageType>
int FW_MessageQueue::SubscribeToMessage(std::function<void(const MessageType&)> aFunction)
{
	FW_MessageBus<MessageType>& bus = GetMessageBus<MessageType>();
	return bus.Subscribe(aFunction);
}

template <typename MessageType>
void FW_MessageQueue::QueueMessage(const MessageType& aMessage)
{
	FW_MessageBus<MessageType>& bus = GetMessageBus<MessageType>();
	bus.QueueMessage(aMessage);
}

template <typename MessageType>
void FW_MessageQueue::InstantlySendMessage(const MessageType& aMessage)
{
	FW_MessageBus<MessageType>& bus = GetMessageBus<MessageType>();
	bus.InstantlySendMessage(aMessage);
}

template <typename MessageType>
FW_MessageBus<MessageType>& FW_MessageQueue::GetMessageBus()
{
	int messageTypeID = FW_TypeID<BaseMessage>::GetID<MessageType>();
	FW_IMessageBus* bus = myMessageBusses[messageTypeID];
	FW_ASSERT(bus != nullptr, "Unregistered MessageType!");

	return *static_cast<FW_MessageBus<MessageType>*>(bus);
}