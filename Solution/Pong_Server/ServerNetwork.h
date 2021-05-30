#pragma once

#include "Network.h"

class ServerNetwork : public Network
{
public:
	template<typename Message>
	void SendNetworkMessage(const Message& aMessage, const sockaddr_in& aTargetAddress);

protected:
	void OnStart() override;
};

template <typename Message>
void ServerNetwork::SendNetworkMessage(const Message& aMessage, const sockaddr_in& aTargetAddress)
{
	NetworkSerializationStreamType messageStream;
	SerializeNetworkMessageHeader(GetNetworkMessageID<Message>(), messageStream);

	aMessage.SerializeMessage(messageStream);
	SendNetworkMessageInternal(messageStream, aTargetAddress);
}
