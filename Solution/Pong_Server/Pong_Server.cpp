#include "Pong_Server.h"
#include "FW_Input.h"

Pong_Server::Pong_Server()
{
	myNetwork.Start();

	myNetwork.RegisterMessageType<ConnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ColorNetworkMessage>();

	myNetwork.SubscribeToMessage<ConnectionNetworkMessage>(std::bind(&Pong_Server::HandleConnectionMessage, this, std::placeholders::_1, std::placeholders::_2));

	myHasAConnectedClient = false;
}

bool Pong_Server::Run()
{
	myNetwork.ProcessMessages();

	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::W))
		BroadcastColorMessage(0xFFFFFFFF);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::A))
		BroadcastColorMessage(0xFFFF0000);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::S))
		BroadcastColorMessage(0xFF00FF00);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::D))
		BroadcastColorMessage(0xFF0000FF);

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

