#include "Pong_Server.h"
#include "FW_Input.h"
#include "FW_Time.h"

Pong_Server::Pong_Server()
{
	myNetwork.Start();

	myNetwork.RegisterMessageType<ClientConnectionRequestNetworkMessage>();
	myNetwork.RegisterMessageType<ClientDisconnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ServerAcceptConnectionNetworkMessage>();

	myNetwork.RegisterMessageType<PlayerSyncNetworkMessage>();

	myNetwork.SubscribeToMessage<ClientConnectionRequestNetworkMessage>(std::bind(&Pong_Server::HandleClientConnectionRequest, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<ClientDisconnectionNetworkMessage>(std::bind(&Pong_Server::HandleClientDisconnection, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<PlayerSyncNetworkMessage>(std::bind(&Pong_Server::HandlePlayerSync, this, std::placeholders::_1, std::placeholders::_2));

	myNextPlayerID = 0;
}

bool Pong_Server::Run()
{
	myNetwork.ProcessMessages();

	return true;
}

void Pong_Server::HandleClientConnectionRequest(const ClientConnectionRequestNetworkMessage& /*aMessage*/, const sockaddr_in& aSenderAddress)
{
	Pong_Player newPlayer;
	newPlayer.myID = myNextPlayerID++;
	newPlayer.myAddress = aSenderAddress;

	if (myPlayers.size() == 0)
	{
		newPlayer.myPosition.x = 200.f;
		newPlayer.myPosition.y = 200.f;
	}
	else
	{
		newPlayer.myPosition.x = 600.f;
		newPlayer.myPosition.y = 200.f;
	}

	myNetwork.SendNetworkMessage(ServerAcceptConnectionNetworkMessage{ newPlayer.myID, newPlayer.myPosition }, newPlayer.myAddress);

	PlayerSyncNetworkMessage newPlayerSync = { newPlayer.myID, newPlayer.myPosition };
	NetworkSerializationStreamType packedNewPlayerMessage;
	myNetwork.PackMessage(newPlayerSync, packedNewPlayerMessage);

	for (const Pong_Player& remotePlayer : myPlayers)
	{
		// Sync the new player to the the remote player
		myNetwork.SendPackedNetworkMessage(packedNewPlayerMessage, remotePlayer.myAddress);

		// Sync the remote player to the new player
		myNetwork.SendNetworkMessage(PlayerSyncNetworkMessage{ remotePlayer.myID, remotePlayer.myPosition }, newPlayer.myAddress);
	}

	myPlayers.push_back(newPlayer);
}

void Pong_Server::HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for (unsigned int i = 0; i < myPlayers.size(); ++i)
	{
		if (myPlayers[i].myID == aMessage.myID)
		{
			myPlayers.erase(myPlayers.begin() + i);
			break;
		}
	}

	ClientDisconnectionNetworkMessage message{ aMessage.myID };
	NetworkSerializationStreamType packedMessage;
	myNetwork.PackMessage(aMessage, packedMessage);

	for (const Pong_Player& remotePlayer : myPlayers)
		myNetwork.SendPackedNetworkMessage(packedMessage, remotePlayer.myAddress);
}

void Pong_Server::HandlePlayerSync(const PlayerSyncNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	NetworkSerializationStreamType packedPlayerUpdate;

	for (Pong_Player& player : myPlayers)
	{
		if (player.myID == aMessage.myID)
		{
			player.myPosition = aMessage.myPosition;

			PlayerSyncNetworkMessage newPlayerSync = { player.myID, player.myPosition };
			myNetwork.PackMessage(newPlayerSync, packedPlayerUpdate);
			break;
		}
	}

	for (const Pong_Player& player : myPlayers)
	{
		if (player.myID != aMessage.myID)
		{
			myNetwork.SendPackedNetworkMessage(packedPlayerUpdate, player.myAddress);
		}
	}
}