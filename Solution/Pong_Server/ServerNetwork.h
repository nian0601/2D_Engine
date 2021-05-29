#pragma once

#include "Network.h"

class ServerNetwork : public Network
{
public:
	void SendNetworkMessage(const char* aMessage, int aSize, const sockaddr_in& aTargetAddress);

protected:
	void OnStart() override;
};
