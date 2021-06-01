#include "Pong_Client.h"
#include "FW_Renderer.h"
#include "FW_Input.h"
#include "FW_Time.h"

Pong_Client::Pong_Client()
{
	myNetwork.Start();

	myNetwork.RegisterMessageType<ClientConnectionRequestNetworkMessage>();
	myNetwork.RegisterMessageType<ClientDisconnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ServerAcceptConnectionNetworkMessage>();

	myNetwork.RegisterMessageType<PlayerSyncNetworkMessage>();

	myNetwork.SubscribeToMessage<ServerAcceptConnectionNetworkMessage>(std::bind(&Pong_Client::HandleServerAcceptedConnection, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<ClientDisconnectionNetworkMessage>(std::bind(&Pong_Client::HandleClientDisconnection, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<PlayerSyncNetworkMessage>(std::bind(&Pong_Client::HandlePlayerSync, this, std::placeholders::_1, std::placeholders::_2));

	myLocalPlayer.myID = -1;
}

bool Pong_Client::Run()
{
	myNetwork.ProcessMessages();

	if (myLocalPlayer.myID != -1)
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

		if (direction != Vector2f())
		{
			float deltaTime = FW_Time::GetDelta();
			float speed = 50.f;
			Normalize(direction);
			myLocalPlayer.myPosition += direction * speed * deltaTime;

			myNetwork.SendNetworkMessage(PlayerSyncNetworkMessage{ myLocalPlayer.myID, myLocalPlayer.myPosition });
		}

		RenderPlayer(myLocalPlayer, 0xFFFFFFFF);
	}
	else
	{
		FW_Renderer::RenderText("Press E to Connect", { 400, 400 });
		if (FW_Input::WasKeyReleased(FW_Input::E))
		{
			ClientConnectionRequestNetworkMessage connectMessage;
			myNetwork.SendNetworkMessage(connectMessage);
		}
	}

	for (const Pong_Player& removePlayer : myRemotePlayers)
		RenderPlayer(removePlayer, 0xFFFF00FF);
		

	return true;
}

void Pong_Client::OnShutdown()
{
	myNetwork.SendNetworkMessage(ClientDisconnectionNetworkMessage{ myLocalPlayer.myID });
}

void Pong_Client::RenderPlayer(const Pong_Player& aPlayer, int aColor)
{
	Rectf rect = MakeRect<float>({ aPlayer.myPosition }, { 32.f, 32.f });
	FW_Renderer::RenderRect(rect, aColor);
}

void Pong_Client::HandleServerAcceptedConnection(const ServerAcceptConnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	myLocalPlayer.myID = aMessage.myLocalPlayer.myID;
	myLocalPlayer.myPosition = aMessage.myLocalPlayer.myPosition;
}

void Pong_Client::HandleClientDisconnection(const ClientDisconnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	for (unsigned int i = 0; i < myRemotePlayers.size(); ++i)
	{
		if (myRemotePlayers[i].myID == aMessage.myID)
		{
			myRemotePlayers.erase(myRemotePlayers.begin() + i);
			break;
		}
	}
}

void Pong_Client::HandlePlayerSync(const PlayerSyncNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	if (myLocalPlayer.myID == aMessage.myID)
	{
		myLocalPlayer.myPosition = aMessage.myPosition;
		return;
	}

	for (Pong_Player& remotePlayer : myRemotePlayers)
	{
		if (remotePlayer.myID == aMessage.myID)
		{
			remotePlayer.myPosition = aMessage.myPosition;
			return;
		}
	}

	Pong_Player player;
	player.myID = aMessage.myID;
	player.myPosition = aMessage.myPosition;
	myRemotePlayers.push_back(player);
}
