#pragma once

#include "SerializationHelper.h"

// Are these really necessary? Is there a neater non-macro solution?
#define SERIALIZE(aStream, aType) serialize(aType,aStream);
#define DESERIALIZE(aStream, aType) aType = deserialize<decltype(aType)>(aStream)

enum MessageType
{
	CLIENT_CONNECT_REQUEST,
	SERVER_ACCEPT_CONNECT_REQUEST,
	SERVER_REJECT_CONNECT_REQUEST,
	CLIENT_DISCONNECT,
	CLIENT_PING,
	SERVER_PING_RESPONSE,
};

struct NetworkMessage
{
	void PackMessage()
	{
		SERIALIZE(myStream, myMessageType);
	}

	void UnPackMessage(char* aBuffer, int aSize)
	{
		if (aSize > 0)
		{
			for (int i = 0; i < aSize; ++i)
			{
				myStream.push_back(aBuffer[i]);
			}
		
			DESERIALIZE(myStream, myMessageType);
		}
	}

	NetworkSerializationStreamType myStream;

	int myMessageType;
};