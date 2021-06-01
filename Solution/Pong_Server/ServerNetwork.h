#pragma once

#include "Network.h"

class ServerNetwork : public Network
{
public:
	template<typename Message>
	void PackMessage(const Message& aMessage, NetworkSerializationStreamType& aMessageStream);

	template<typename Message>
	void SendNetworkMessage(const Message& aMessage, const sockaddr_in& aTargetAddress);

protected:
	void OnStart() override;
};

template <typename Message>
void ServerNetwork::SendNetworkMessage(const Message& aMessage, const sockaddr_in& aTargetAddress)
{
	NetworkSerializationStreamType messageStream;
	PackMessage(aMessage, messageStream);
	SendPackedNetworkMessage(messageStream, aTargetAddress);
}

template <typename Message>
void ServerNetwork::PackMessage(const Message& aMessage, NetworkSerializationStreamType& aMessageStream)
{
	SerializeNetworkMessageHeader(GetNetworkMessageID<Message>(), aMessageStream);
	aMessage.SerializeMessage(aMessageStream);
}