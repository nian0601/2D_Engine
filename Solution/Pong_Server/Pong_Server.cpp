#include "Pong_Server.h"
#include "FW_Input.h"
#include "FW_Time.h"

Pong_Server::Pong_Server()
{
	myNetwork.Start();

	myNetwork.RegisterMessageType<ConnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ColorNetworkMessage>();
	myNetwork.RegisterMessageType<PositionNetworkMessage>();

	myNetwork.SubscribeToMessage<ConnectionNetworkMessage>(std::bind(&Pong_Server::HandleConnectionMessage, this, std::placeholders::_1, std::placeholders::_2));

	myHasAConnectedClient = false;
	myPosition = { 400.f, 400.f };
}

bool Pong_Server::Run()
{
	myNetwork.ProcessMessages();

	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_1))
		BroadcastColorMessage(0xFFFFFFFF);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_2))
		BroadcastColorMessage(0xFFFF0000);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_3))
		BroadcastColorMessage(0xFF00FF00);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_4))
		BroadcastColorMessage(0xFF0000FF);


	const float aSpeed = 10.f;
	float deltaTime = FW_Time::GetDelta();
	if (FW_Input::IsKeyDown(FW_Input::KeyCode::W))
		myPosition.y -= aSpeed * deltaTime;
	if (FW_Input::IsKeyDown(FW_Input::KeyCode::S))
		myPosition.y += aSpeed * deltaTime;
	if (FW_Input::IsKeyDown(FW_Input::KeyCode::A))
		myPosition.x -= aSpeed * deltaTime;
	if (FW_Input::IsKeyDown(FW_Input::KeyCode::D))
		myPosition.x += aSpeed * deltaTime;

	if (myConnectedClients.size() > 0)
	{
		PositionNetworkMessage message;
		message.myPosition = myPosition;

		for (const sockaddr_in& client : myConnectedClients)
			myNetwork.SendNetworkMessage(message, client);
	}

	return true;
}

void Pong_Server::HandleConnectionMessage(const ConnectionNetworkMessage& aMessage, const sockaddr_in& aSender)
{
	switch (aMessage.myConnectionType)
	{
	case ConnectionNetworkMessage::CLIENT_CONNECT_REQUEST:
	{
		myHasAConnectedClient = true;

		ConnectionNetworkMessage response;
		response.myConnectionType = ConnectionNetworkMessage::SERVER_ACCEPT_CONNECT_REQUEST;
		myNetwork.SendNetworkMessage(response, aSender);

		myConnectedClients.push_back(aSender);
		break;
	}
	}
}

void Pong_Server::BroadcastColorMessage(int aColor)
{
	ColorNetworkMessage response;
	response.myColor = aColor;

	for (const sockaddr_in& client : myConnectedClients)
		myNetwork.SendNetworkMessage(response, client);
}

