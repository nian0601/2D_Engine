#pragma once

#include "FW_IGame.h"

#include "ClientNetwork.h"

class Pong_Client : public FW_IGame
{
public:
	Pong_Client();

	bool Run() override;

private:
	ClientNetwork myNetwork;
	std::vector<NetworkData> myNetworkData;
	bool myIsConnectedToServer;
};
