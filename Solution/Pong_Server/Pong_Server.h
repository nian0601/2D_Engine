#pragma once

#include "FW_IGame.h"
#include "ServerNetwork.h"

class Pong_Server : public FW_IGame
{
public:
	Pong_Server();

	bool Run() override;

private:
	ServerNetwork myNetwork;
	std::vector<NetworkData> myNetworkData;
	bool myHasAConnectedClient;
};
