#pragma once

#include "FW_IGame.h"

#include "ClientNetwork.h"

class Pong_Client : public FW_IGame
{
public:
	Pong_Client();

	bool Run() override;

private:
	void HandleConnectionMessage(const ConnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleColorMessage(const ColorNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandlePositionMessage(const PositionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);

	ClientNetwork myNetwork;
	bool myIsConnectedToServer;
	int myColor;
	Vector2f myPosition;
};
