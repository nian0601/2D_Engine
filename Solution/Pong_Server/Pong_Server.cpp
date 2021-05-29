#include "Pong_Server.h"

Pong_Server::Pong_Server()
{
	myNetwork.Start();
	myHasAConnectedClient = false;
}

bool Pong_Server::Run()
{
	if (myNetwork.RecieveMessages(myNetworkData))
	{
		NetworkMessage message;
		for (NetworkData& networkData : myNetworkData)
		{
			message.UnPackMessage(networkData.myData, networkData.myLength);

			switch (message.myMessageType)
			{
			case CLIENT_CONNECT_REQUEST:
			{
				myHasAConnectedClient = true;

				NetworkMessage response;
				response.myMessageType = SERVER_ACCEPT_CONNECT_REQUEST;
				response.PackMessage();
				myNetwork.SendNetworkMessage(&response.myStream[0], response.myStream.size(), networkData.myAddress);
			}
			}
		}
	}


	return true;
}

