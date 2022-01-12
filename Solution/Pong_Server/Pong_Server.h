#pragma once

#include "FW_IGame.h"
#include "ServerNetwork.h"

struct ConnectedClient
{
	int myEntityID;
	sockaddr_in myAddress;
};

struct Entity
{
	int myID;
	Vector2f myPosition;
	Vector2f myVelocity;
	float mySpeed;
};

class Pong_Server : public FW_IGame
{
public:
	Pong_Server();

	bool Run() override;
	void OnShutdown() override;

	void StartExtraClientProcess(int aWindowX = -1, int aWindowY = -1);

private:
	void HandleClientConnectionRequest(const ClientConnectionRequestNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandlePlayerSync(const PlayerSyncNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	
	Entity& CreateNewEntity();

	ServerNetwork myNetwork;
	int myNextEntityID;
	FW_GrowingArray<Entity> myEntities;
	FW_GrowingArray<ConnectedClient> myClients;
	int myBallID;

	float myElapsedTime;
	int myFrameCount;

	std::vector<PROCESS_INFORMATION> myChildProcesses;
};
