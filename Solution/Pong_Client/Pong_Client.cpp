#include "Pong_Client.h"
#include "..\Framework\FW_Renderer.h"

Pong_Client::Pong_Client()
{
	myNetwork.Start();

	NetworkMessage connectMessage;
	connectMessage.myMessageType = CLIENT_CONNECT_REQUEST;
	connectMessage.PackMessage();

	myNetwork.SendNetworkMessage(&connectMessage.myStream[0], connectMessage.myStream.size());

	myIsConnectedToServer = false;
}

bool Pong_Client::Run()
{
	if (myNetwork.RecieveMessages(myNetworkData))
	{
		NetworkMessage message;
		for (NetworkData& networkData : myNetworkData)
		{
			message.UnPackMessage(networkData.myData, networkData.myLength);

			switch (message.myMessageType)
			{
			case SERVER_ACCEPT_CONNECT_REQUEST:
			{
				myIsConnectedToServer = true;
				break;
			}
			default:
				break;
			}
		}
	}

	if(myIsConnectedToServer)
		FW_Renderer::RenderText("Connected to server!", { 400, 400 });

	return true;
}

