#pragma once

#include "FW_IGame.h"

#include "ClientNetwork.h"

struct RemoteEntityState
{
	int myStateID = -1;
	Vector2f myPosition;
};

struct RemoteEntity
{
	int myEntityID;
	//FW_GrowingArray<RemoteEntityState> myRemoteStates;

	//RemoteEntityState myAlmostMostRecentServerState;
	RemoteEntityState myMostRecentServerState;

	Vector2f myCurrentPosition;
	Vector2f myTargetPosition;
};

class Pong_Client : public FW_IGame
{
public:
	Pong_Client();

	bool Run() override;
	void OnShutdown() override;

private:
	void RenderEntity(const Vector2f& aPosition, int aColor);

	void HandleServerAcceptedConnection(const ServerAcceptConnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleNewPlayerConnected(const NewPlayerConnectedNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleFullSync(const FullSyncNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);
	void HandleServerEntity(const ServerEntityNetworkMessage& aMessage, const sockaddr_in& aSenderAddress);

	void RequestConnectionToServer();

	ClientNetwork myNetwork;

	FW_GrowingArray<RemoteEntity> myEntities;
	int myLocalEntityID;


	bool myEnablePositionInterpolation;
	bool myShowDemoWindow;

	void StartExtraClientProcess();
	void StartServerProcess();
	std::vector<PROCESS_INFORMATION> myChildProcesses;

	Vector2f InterpolateTowardsVector(const Vector2f& aCurrent, const Vector2f& aTarget, float aInterpolationTime, float aDelta) const;
};
