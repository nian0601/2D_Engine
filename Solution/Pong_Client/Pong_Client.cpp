#include "Pong_Client.h"
#include "FW_Renderer.h"

Pong_Client::Pong_Client()
{
	myNetwork.Start();

	myNetwork.RegisterMessageType<ConnectionNetworkMessage>();
	myNetwork.RegisterMessageType<ColorNetworkMessage>();
	myNetwork.RegisterMessageType<PositionNetworkMessage>();

	myNetwork.SubscribeToMessage<ConnectionNetworkMessage>(std::bind(&Pong_Client::HandleConnectionMessage, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<ColorNetworkMessage>(std::bind(&Pong_Client::HandleColorMessage, this, std::placeholders::_1, std::placeholders::_2));
	myNetwork.SubscribeToMessage<PositionNetworkMessage>(std::bind(&Pong_Client::HandlePositionMessage, this, std::placeholders::_1, std::placeholders::_2));

	ConnectionNetworkMessage connectMessage;
	connectMessage.myConnectionType = ConnectionNetworkMessage::CLIENT_CONNECT_REQUEST;
	myNetwork.SendNetworkMessage(connectMessage);

	myIsConnectedToServer = false;
	myColor = 0xFFFFFFFF;
	myPosition = { 400.f, 400.f };
}

bool Pong_Client::Run()
{
	myNetwork.ProcessMessages();

	if(myIsConnectedToServer)
		FW_Renderer::RenderText("Connected to server!", { int(myPosition.x), int(myPosition.y) }, myColor);

	return true;
}

void Pong_Client::HandleConnectionMessage(const ConnectionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	switch (aMessage.myConnectionType)
	{
	case ConnectionNetworkMessage::SERVER_ACCEPT_CONNECT_REQUEST:
	{
		myIsConnectedToServer = true;
		break;
	}
	}
}

void Pong_Client::HandleColorMessage(const ColorNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	myColor = aMessage.myColor;
}

void Pong_Client::HandlePositionMessage(const PositionNetworkMessage& aMessage, const sockaddr_in& /*aSenderAddress*/)
{
	myPosition = aMessage.myPosition;
}

