#pragma once

#include "FW_IGame.h"

#include "ClientNetwork.h"

struct Pong_Player
{
	int myID;
	Vector2f myPosition;
};

class Pong_Client : public FW_IGame
{
public:
	Pong_Client();

	bool Run() override;
	void OnShutdown() override;

private:
	void RenderPlayer(const Pong_Player& aPlayer, int aColor);

	void HandleServerAcceptedConnection(const ServerAcceptConnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandlePlayerSync(const PlayerSyncNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);

	ClientNetwork myNetwork;

	std::vector<Pong_Player> myRemotePlayers;
	Pong_Player myLocalPlayer;
};
