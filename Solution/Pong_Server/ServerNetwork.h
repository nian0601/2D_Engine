#pragma once

#include "Network.h"
#include "FW_GrowingArray.h"

class ServerNetwork : public Network
{
public:
	template<typename Message>
	void PackMessage(const Message& aMessage, NetworkSerializationStreamType& aMessageStream);

	template<typename Message>
	void SendNetworkMessage(const Message& aMessage, const sockaddr_in& aTargetAddress);

	template<typename Message>
	void SendNetworkMessageToAllClients(const Message& aMessage);

	void AddClient(int aID, const sockaddr_in& aAddress);
	void RemoveClient(int aID);

protected:
	void OnStart() override;

	struct ConnectedClient
	{
		int myID;
		sockaddr_in myAddress;
	};
	FW_GrowingArray<ConnectedClient> myConnectedClients;
};

template <typename Message>
void ServerNetwork::SendNetworkMessage(const Message& aMessage, const sockaddr_in& aTargetAddress)
{
	NetworkSerializationStreamType messageStream;
	PackMessage(aMessage, messageStream);
	SendPackedNetworkMessage(messageStream, aTargetAddress);
}

template<typename Message>
void ServerNetwork::SendNetworkMessageToAllClients(const Message& aMessage)
{
	NetworkSerializationStreamType packedMessage;
	PackMessage(aMessage, packedMessage);

	for (const ConnectedClient& client : myConnectedClients)
		SendPackedNetworkMessage(packedMessage, client.myAddress);
}

template <typename Message>
void ServerNetwork::PackMessage(const Message& aMessage, NetworkSerializationStreamType& aMessageStream)
{
	SerializeNetworkMessageHeader(GetNetworkMessageID<Message>(), aMessageStream);
	aMessage.SerializeMessage(aMessageStream);
}