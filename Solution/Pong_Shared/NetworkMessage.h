#pragma once

#include "SerializationHelper.h"
#include "..\Framework\FW_TypeID.h"
#include "..\Framework\FW_Vector2.h"
#include "..\Framework\FW_GrowingArray.h"

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

// This is only used as a holder for the TypeID-system to group all the NetworkMessages together in the same "TypeID Family"
struct BaseNetworkMessage{};

template<typename Message>
int GetNetworkMessageID()
{
	return FW_TypeID<BaseNetworkMessage>::GetID<Message>();
}

struct ClientConnectionRequestNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& /*aStream*/) const {}
	void DeserializeMessage(NetworkSerializationStreamType& /*aStream*/) {}
};

struct ClientDisconnectionNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const 
	{
		SERIALIZE(aStream, myID);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream) 
	{
		DESERIALIZE(aStream, myID);
	}

	int myID;
};

struct ServerAcceptConnectionNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myID);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myID);
	}

	int myID;
};

struct NewPlayerConnectedNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myID);
		SERIALIZE(aStream, myPosition);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myID);
		DESERIALIZE(aStream, myPosition);
	}

	int myID;
	Vector2f myPosition;
};

struct PlayerSyncNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myID);
		SERIALIZE(aStream, myPosition);
		SERIALIZE(aStream, myVelocity);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myID);
		DESERIALIZE(aStream, myPosition);
		DESERIALIZE(aStream, myVelocity);
	}

	int myID;
	Vector2f myPosition;
	Vector2f myVelocity;
};

struct FullSyncNetworkMessage
{
	struct Entity
	{
		int myServerStateID;
		int myEntityID;
		Vector2f myPosition;
		Vector2f myVelocity;
	};

	FW_GrowingArray<Entity> myEntities;

	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myEntities.Count());
		for (const Entity& entity : myEntities)
		{
			SERIALIZE(aStream, entity.myServerStateID);
			SERIALIZE(aStream, entity.myEntityID);
			SERIALIZE(aStream, entity.myPosition);
			SERIALIZE(aStream, entity.myVelocity);
		}
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		int entityCount = 0;
		DESERIALIZE(aStream, entityCount);
		for (int i = 0; i < entityCount; ++i)
		{
			Entity& entity = myEntities.Add();
			DESERIALIZE(aStream, entity.myServerStateID);
			DESERIALIZE(aStream, entity.myEntityID);
			DESERIALIZE(aStream, entity.myPosition);
			DESERIALIZE(aStream, entity.myVelocity);
		}
	}
};


//////////////////////////////////////////////////////////////////////////

struct ServerEntityNetworkMessage
{
	void SerializeMessage(NetworkSerializationStreamType& aStream) const
	{
		SERIALIZE(aStream, myServerStateID);
		SERIALIZE(aStream, myEntityID);
		SERIALIZE(aStream, myPosition);
		SERIALIZE(aStream, myVelocity);
	}

	void DeserializeMessage(NetworkSerializationStreamType& aStream)
	{
		DESERIALIZE(aStream, myServerStateID);
		DESERIALIZE(aStream, myEntityID);
		DESERIALIZE(aStream, myPosition);
		DESERIALIZE(aStream, myVelocity);
	}

	int myServerStateID;
	int myEntityID;
	Vector2f myPosition;
	Vector2f myVelocity;
};