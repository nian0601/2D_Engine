#pragma once

#include "Network.h"

class ClientNetwork : public Network
{
public:
	void SendNetworkMessage(const char* aMessage, int aSize);

protected:
	void OnStart() override;

	sockaddr_in myServerAddress;
};
