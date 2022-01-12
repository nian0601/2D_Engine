#include "Pong_Server.h"
#include "FW_Input.h"
#include "FW_Time.h"
#include "FW_String.h"
#include "FW_Logger.h"

Pong_Server::Pong_Server()
{
	myNetwork.Start();

	myNetwork.RegisterMessageType<ClientConnectionRequestNetworkMessage>();
	myNetwork.RegisterMessageType<ClientDisconnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ServerAcceptConnectionNetworkMessage>();
	myNetwork.RegisterMessageType<NewPlayerConnectedNetworkMessage>();

	myNetwork.RegisterMessageType<PlayerSyncNetworkMessage>();
	myNetwork.RegisterMessageType<FullSyncNetworkMessage>();
	myNetwork.RegisterMessageType<ServerEntityNetworkMessage>();

	myNetwork.SubscribeToMessage<ClientConnectionRequestNetworkMessage>(std::bind(&Pong_Server::HandleClientConnectionRequest, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<ClientDisconnectionNetworkMessage>(std::bind(&Pong_Server::HandleClientDisconnection, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<PlayerSyncNetworkMessage>(std::bind(&Pong_Server::HandlePlayerSync, this, std::placeholders::_1, std::placeholders::_2));

#if 1
	// Client-size, Should be specified in some clever shared place in the future
	// 720, 640
	//StartExtraClientProcess(200, 400);
	//StartExtraClientProcess(1200, 400);
#endif

	myElapsedTime = 0.f;
	myFrameCount = 0;

	myNextEntityID = 0;
	Entity& ball = CreateNewEntity();
	ball.myPosition = { 360.f, 320 };
	ball.myVelocity = { 100.f, 50.f };
	Normalize(ball.myVelocity);
	ball.myVelocity *= 200.f;
	myBallID = ball.myID;
}

bool Pong_Server::Run()
{
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_F2))
		StartExtraClientProcess();

	myNetwork.ProcessMessages();

	static float timeLimiter = 1.f / 10.f;
	float deltaTime = FW_Time::GetDelta();

	myElapsedTime += deltaTime;
	if (myElapsedTime < timeLimiter)
		return true;

	for (Entity& entity : myEntities)
	{
		entity.myPosition += entity.myVelocity * myElapsedTime;

		if (entity.myID == myBallID)
		{
			if (entity.myPosition.y <= 0 && entity.myVelocity.y < 0)
				entity.myVelocity.y *= -1.f;
			else if (entity.myPosition.y >= 640 && entity.myVelocity.y > 0)
				entity.myVelocity.y *= -1.f;

			if (entity.myPosition.x <= 0 && entity.myVelocity.x < 0)
				entity.myVelocity.x *= -1.f;
			if (entity.myPosition.x >= 720 && entity.myVelocity.x > 0)
				entity.myVelocity.x *= -1.f;
		}

		ServerEntityNetworkMessage ballMessage = { myFrameCount, entity.myID, entity.myPosition, entity.myVelocity };
		myNetwork.SendNetworkMessageToAllClients(ballMessage);
	}

	myElapsedTime = 0.f;
	++myFrameCount;

	return true;
}

void Pong_Server::OnShutdown()
{
	for (const PROCESS_INFORMATION& childProcess : myChildProcesses)
	{
		TerminateProcess(childProcess.hProcess, 0);
		CloseHandle(childProcess.hProcess);
		CloseHandle(childProcess.hThread);
	}
}

void Pong_Server::HandleClientConnectionRequest(const ClientConnectionRequestNetworkMessage& /*aMessage*/, const sockaddr_in& aSenderAddress)
{
	Entity& playerEntity = CreateNewEntity();;
	if (myClients.IsEmpty())
	{
		playerEntity.myPosition.x = 200.f;
		playerEntity.myPosition.y = 200.f;
	}
	else
	{
		playerEntity.myPosition.x = 600.f;
		playerEntity.myPosition.y = 200.f;
	}

	ConnectedClient& client = myClients.Add();
	client.myAddress = aSenderAddress;
	client.myEntityID = playerEntity.myID;


	// Tell the new player that the connection was accepted and give the client its ID
	myNetwork.SendNetworkMessage(ServerAcceptConnectionNetworkMessage{ client.myEntityID }, client.myAddress);

	// Then send the current state of the game to the player
	FullSyncNetworkMessage newPlayerFullSync;
	for (Entity& entity : myEntities)
		newPlayerFullSync.myEntities.Add({ myFrameCount, entity.myID, entity.myPosition, entity.myVelocity });

	myNetwork.SendNetworkMessage(newPlayerFullSync, client.myAddress);

	// Send the new player to all existing players
	NewPlayerConnectedNetworkMessage newPlayerSync = { playerEntity.myID, playerEntity.myPosition };
	myNetwork.SendNetworkMessageToAllClients(newPlayerSync);

	// Finally add the new client to the lists so that its included in any future "server-wide" messages
	myNetwork.AddClient(client.myEntityID, client.myAddress);

	FW_Logger::AddMessage("Player Connected");
}

void Pong_Server::HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for (int i = 0; i < myClients.Count(); ++i)
	{
		if (myClients[i].myEntityID == aMessage.myID)
		{
			myClients.RemoveCyclicAtIndex(i);
			myNetwork.RemoveClient(aMessage.myID);

			FW_Logger::AddMessage("Player Disconnected");
			break;
		}
	}

	ClientDisconnectionNetworkMessage message{ aMessage.myID };
	myNetwork.SendNetworkMessageToAllClients(message);
}

void Pong_Server::HandlePlayerSync(const PlayerSyncNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for (Entity& entity : myEntities)
	{
		if (entity.myID == aMessage.myID)
		{
			entity.myPosition = aMessage.myPosition;
			entity.myVelocity = aMessage.myVelocity;
			break;
		}
	}
}

Entity& Pong_Server::CreateNewEntity()
{
	Entity& entity = myEntities.Add();
	entity.myID = myNextEntityID++;
	return entity;
}

void Pong_Server::StartExtraClientProcess(int aWindowX, int aWindowY)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	FW_String cmdArgs = " "; // Blankspace first as a dummy for the application-name
	cmdArgs += aWindowX;
	cmdArgs += " ";
	cmdArgs += aWindowY;


	// Start the child process. 
	if (!CreateProcess("c://_GIT//2D_Engine//Pong_Client//Pong_Client.exe",   // No module name (use command line)
		cmdArgs.GetRawBuffer(), // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
	}
	else
	{
		myChildProcesses.push_back(pi);
		FW_Logger::AddMessage("New Client-process started");
	}
}