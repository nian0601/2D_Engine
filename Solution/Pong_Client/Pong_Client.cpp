#include "Pong_Client.h"
#include "FW_Renderer.h"
#include "FW_Input.h"
#include "FW_Time.h"
#include "FW_String.h"

#include "imgui/imgui.h"
#include "FW_Logger.h"

Pong_Client::Pong_Client()
{
	myShowDemoWindow = false;

	myNetwork.Start();

	myNetwork.RegisterMessageType<ClientConnectionRequestNetworkMessage>();
	myNetwork.RegisterMessageType<ClientDisconnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ServerAcceptConnectionNetworkMessage>();
	myNetwork.RegisterMessageType<NewPlayerConnectedNetworkMessage>();

	myNetwork.RegisterMessageType<PlayerSyncNetworkMessage>();
	myNetwork.RegisterMessageType<FullSyncNetworkMessage>();
	myNetwork.RegisterMessageType<ServerEntityNetworkMessage>();

	myNetwork.SubscribeToMessage<ServerAcceptConnectionNetworkMessage>(std::bind(&Pong_Client::HandleServerAcceptedConnection, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<ClientDisconnectionNetworkMessage>(std::bind(&Pong_Client::HandleClientDisconnection, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<NewPlayerConnectedNetworkMessage>(std::bind(&Pong_Client::HandleNewPlayerConnected, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<FullSyncNetworkMessage>(std::bind(&Pong_Client::HandleFullSync, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<ServerEntityNetworkMessage>(std::bind(&Pong_Client::HandleServerEntity, this, std::placeholders::_1, std::placeholders::_2));

	myLocalEntityID = -1;
	myEnablePositionInterpolation = true;

	RequestConnectionToServer();
}

bool Pong_Client::Run()
{
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_F1))
		StartServerProcess();
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_F2))
		StartExtraClientProcess();

	myNetwork.ProcessMessages();

	float deltaTime = FW_Time::GetDelta();

	if (myLocalEntityID == -1)
	{
		FW_Renderer::RenderText("Press E to Connect", { 400, 400 });
		if (FW_Input::WasKeyReleased(FW_Input::E))
			RequestConnectionToServer();
	}

	for (RemoteEntity& entity : myEntities)
	{
		int color = 0xFFFF00FF;

		if (entity.myEntityID == myLocalEntityID)
		{
			Vector2f direction;
			if (FW_Input::IsKeyDown(FW_Input::KeyCode::W))
				direction.y = -1.f;
			if (FW_Input::IsKeyDown(FW_Input::KeyCode::S))
				direction.y = 1.f;
			if (FW_Input::IsKeyDown(FW_Input::KeyCode::A))
				direction.x = -1.f;
			if (FW_Input::IsKeyDown(FW_Input::KeyCode::D))
				direction.x = 1.f;

			float speed = 50.f;
			Normalize(direction);
			entity.myCurrentPosition += direction * speed * deltaTime;

			myNetwork.SendNetworkMessage(PlayerSyncNetworkMessage{ entity.myEntityID, entity.myCurrentPosition, direction * speed });

			color = 0xFFFFFFFF;
		}
		else
		{
			entity.myCurrentPosition = InterpolateTowardsVector(entity.myCurrentPosition, entity.myTargetPosition, 0.1f, deltaTime);
		}
			

		RenderEntity(entity.myCurrentPosition, color);
	}

	//FW_Time::ClampToFramerate(1.f / 60.f);

	ImGui::Begin("Pong Client");
	ImGui::Checkbox("Enable Interpolation", &myEnablePositionInterpolation);
	ImGui::End();

	return true;
}

void Pong_Client::OnShutdown()
{
	myNetwork.SendNetworkMessage(ClientDisconnectionNetworkMessage{ myLocalEntityID });

	for (const PROCESS_INFORMATION& childProcess : myChildProcesses)
	{
		TerminateProcess(childProcess.hProcess, 0);
		CloseHandle(childProcess.hProcess);
		CloseHandle(childProcess.hThread);
	}

	FW_Logger::AddMessage("Sent Disconnect-message");
}

void Pong_Client::RenderEntity(const Vector2f& aPosition, int aColor)
{
	Rectf rect = MakeRect<float>({ aPosition }, { 32.f, 32.f });
	FW_Renderer::RenderRect(rect, aColor);
}

void Pong_Client::HandleServerAcceptedConnection(const ServerAcceptConnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	myLocalEntityID = aMessage.myID;
	FW_Logger::AddMessage("ConnectionRequest Accepted");
}

void Pong_Client::HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for(int i = 0; i < myEntities.Count(); ++i)
	{
		if (myEntities[i].myEntityID == aMessage.myID)
		{
			myEntities.RemoveCyclicAtIndex(i);
			break;
		}
	}
}

void Pong_Client::HandleNewPlayerConnected(const NewPlayerConnectedNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	RemoteEntity entity;
	entity.myEntityID = aMessage.myID;
	entity.myMostRecentServerState.myStateID = 0;
	entity.myMostRecentServerState.myPosition = aMessage.myPosition;

	entity.myCurrentPosition = entity.myMostRecentServerState.myPosition;
	entity.myTargetPosition = entity.myCurrentPosition;

	myEntities.Add(entity);
}

void Pong_Client::HandleFullSync(const FullSyncNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for (const FullSyncNetworkMessage::Entity& networkEntity : aMessage.myEntities)
	{
		RemoteEntity entity;
		entity.myEntityID = networkEntity.myEntityID;
		entity.myMostRecentServerState.myStateID = networkEntity.myServerStateID;
		entity.myMostRecentServerState.myPosition = networkEntity.myPosition;

		entity.myCurrentPosition = entity.myMostRecentServerState.myPosition;
		entity.myTargetPosition = entity.myCurrentPosition;
	
		myEntities.Add(entity);
	}
}

void Pong_Client::HandleServerEntity(const ServerEntityNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for (int i = 0; i < myEntities.Count(); ++i)
	{
		if (myEntities[i].myEntityID == aMessage.myEntityID)
		{
			RemoteEntity& entity = myEntities[i];
			if (aMessage.myServerStateID > entity.myMostRecentServerState.myStateID)
			{
				entity.myMostRecentServerState.myStateID = aMessage.myServerStateID;
				entity.myMostRecentServerState.myPosition = aMessage.myPosition;

				entity.myTargetPosition = entity.myMostRecentServerState.myPosition;
			}

			return;
		}
	}
}

void Pong_Client::RequestConnectionToServer()
{
	ClientConnectionRequestNetworkMessage connectMessage;
	myNetwork.SendNetworkMessage(connectMessage);
	FW_Logger::AddMessage("ConnectionRequest Sent");
}

void Pong_Client::StartExtraClientProcess()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));


	// Start the child process. 
	if (!CreateProcess("c://_GIT//2D_Engine//Pong_Client//Pong_Client.exe",   // No module name (use command line)
		NULL,			// Command line
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
	}

}

void Pong_Client::StartServerProcess()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	FW_String cmdArgs = " "; // Blankspace first as a dummy for the application-name
	cmdArgs += 0; //0 == Server *WONT* start extra clients, 1 == Server *WILL* start extra clients on startup

	// Start the child process. 
	if (!CreateProcess("c://_GIT//2D_Engine//Pong_Server//Pong_Server.exe",   // No module name (use command line)
		cmdArgs.GetRawBuffer(),			// Command line
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
	}
}

Vector2f Pong_Client::InterpolateTowardsVector(const Vector2f& aCurrent, const Vector2f& aTarget, float aInterpolationTime, float aDelta) const
{
	if (!myEnablePositionInterpolation)
		return aTarget;


	Vector2f direction = aTarget - aCurrent;
	if (Length2(direction) < 0.01f)
		return aTarget;

	direction /= aInterpolationTime;

	return aCurrent + direction * aDelta;
}
