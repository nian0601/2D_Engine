#pragma once

#include "FW_IGame.h"
#include "ServerNetwork.h"

class Pong_Server : public FW_IGame
{
public:
	Pong_Server();

	bool Run() override;

private:
	void HandleConnectionMessage(const ConnectionNetworkMessage& aMessage, const sockaddr_in& aSender);
	void BroadcastColorMessage(int aColor);

	ServerNetwork myNetwork;
	bool myHasAConnectedClient;

	std::vector<sockaddr_in> myConnectedClients;

	Vector2f myPosition;
};
