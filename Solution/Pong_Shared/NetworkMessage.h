#pragma once

#include "SerializationHelper.h"
#include "..\Framework\FW_TypeID.h"
#include "..\Framework\FW_Vector2.h"

// Are these really necessary? Is there a neater non-macro solution?
#define SERIALIZE(aStream, aType) serialize(aType,aStream);
#define DESERIALIZE(aStream, aType) aType = deserialize<decltype(aType)>(aStream)

struct NetworkData
{
	char myData[512];
	int myLength;
	sockaddr_in myAddress;
};

static const int GAME_ID = 1234;
static const int SUPERSECRET_HASH = 5678;

inline void SerializeNetworkMessageHeader(int aMessageType, NetworkSerializationStreamType& aStream)
{
	SERIALIZE(aStream, GAME_ID); // [0-3]
	SERIALIZE(aStream, SUPERSECRET_HASH); // [4-7]
	SERIALIZE(aStream, aMessageType); // [8-11]
}

// Returns the MessageType as an int
inline int InitializeMessageStreamFromNetworkData(NetworkSerializationStreamType& aStream, NetworkData& someNetworkData)
{
	if (someNetworkData.myLength > 0)
	{
		for (int i = 0; i < someNetworkData.myLength; ++i)
			aStream.push_back(someNetworkData.myData[i]);
	}

	int gameID;
	DESERIALIZE(aStream, gameID); // [0-3]
	assert(gameID == GAME_ID);

	int superSecretHash;
	DESERIALIZE(aStream, superSecretHash); // [4-7]
	assert(superSecretHash == SUPERSECRET_HASH);

	int messageType;
	DESERIALIZE(aStream, messageType); // [8-11]
	return messageType;
}

// This is only used as a holder for the TypeID-system to group all the NetworkMessages together in the same "TypeID Familiy"
struct BaseNetworkMessage {};

template<typename Message>
int GetNetworkMessageID()
{
	return FW_TypeID<BaseNetworkMessage>::GetID<Message>();
}

struct ConnectionNetworkMessage
{
	enum MessageType
	{
		CLIENT_CONNECT_REQUEST,
		SERVER_ACCEPT_CONNECT_REQUEST,
		SERVER_REJECT_CONNECT_REQUEST,
		CLIENT_DISCONNECT,
	};

	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myConnectionType);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myConnectionType);
	}

	int myConnectionType;
};

struct ColorNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myColor);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myColor);
	}

	int myColor;
};

struct PositionNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myPosition.x);
		SERIALIZE(aStream, myPosition.y);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myPosition.x);
		DESERIALIZE(aStream, myPosition.y);
	}

	Vector2f myPosition;
};