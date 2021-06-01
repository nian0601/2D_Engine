#pragma once

#include "Network.h"

class ClientNetwork : public Network
{
public:
	template<typename Message>
	void SendNetworkMessage(const Message& aMessage);

protected:
	void OnStart() override;

	sockaddr_in myServerAddress;
};

template <typename Message>
void ClientNetwork::SendNetworkMessage(const Message& aMessage)
{
	NetworkSerializationStreamType messageStream;
	SerializeNetworkMessageHeader(GetNetworkMessageID<Message>(), messageStream);

	aMessage.SerializeMessage(messageStream);
	SendPackedNetworkMessage(messageStream, myServerAddress);
}

