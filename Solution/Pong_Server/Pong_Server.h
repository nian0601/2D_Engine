#pragma once

#include "FW_IGame.h"
#include "ServerNetwork.h"

struct Pong_Player
{
	int myID = 0;

	Vector2f myPosition;
	sockaddr_in myAddress;
};

class Pong_Server : public FW_IGame
{
public:
	Pong_Server();

	bool Run() override;

private:
	void HandleClientConnectionRequest(const ClientConnectionRequestNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandlePlayerSync(const PlayerSyncNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	
	ServerNetwork myNetwork;

	int myNextPlayerID;
	std::vector<Pong_Player> myPlayers;
};
