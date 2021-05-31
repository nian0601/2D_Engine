#include "Network.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <thread>


Network::Network()
	: myIsRunning(false)
	, mySendAndReceiveThread(nullptr)
{
}

Network::~Network()
{
	myIsRunning = false;

	mySendAndReceiveThread->join();
	delete mySendAndReceiveThread;

	closesocket(mySocket);
	WSACleanup();
}

void Network::Start()
{
	OnStart();

#if THREAD_MODE
	myIsRunning = true;
	mySendAndReceiveThread = new std::thread([&] {SendAndReceiveThreadFunction(); });
#endif
}

void Network::ProcessMessages()
{
	myNetworkData.clear();

#if THREAD_MODE
	{
		// If we're running in threaded mode then we need to lock the mutex before accessing the Buffer
		// We dont want to keep the mutex locked while we parse the messages, convert them to GameMessages and send those out,
		// since we dont have any control over how long it will take for the game to handle each message.
		// So instead we just copy over the raw network-data into a "game list" that we then can process without needing the lock
		ReadWriteLock lock(mySendAndReceiveMutex);
		for (const NetworkData& data : myIncommingNetworkDataBuffer)
			myNetworkData.push_back(data);

		myIncommingNetworkDataBuffer.clear();
	}
#else
	int toReturn = 0;
	int size = sizeof(sockaddr_in);

	const int bufferSize = 512;
	char buffer[bufferSize];
	ZeroMemory(&buffer, bufferSize);

	NetworkData dataToAdd;
	while ((toReturn = recvfrom(mySocket, buffer, bufferSize, 0, (struct sockaddr *)&dataToAdd.myAddress, &size)) > 0)
	{
		memcpy(&dataToAdd.myData, &buffer[0], toReturn * sizeof(char));
		dataToAdd.myLength = toReturn;
		myNetworkData.push_back(dataToAdd);
	}
#endif


	if (!myNetworkData.empty())
	{
		for (NetworkData& networkData : myNetworkData)
		{
			NetworkSerializationStreamType messageStream;
			int messageType = InitializeMessageStreamFromNetworkData(messageStream, networkData);

			SendMessage(messageType, messageStream, networkData.myAddress);
		}
	}
}

void Network::SendNetworkMessageInternal(const NetworkSerializationStreamType& someData, const sockaddr_in& aTargetAddress)
{
#if THREAD_MODE
	NetworkData data;
	memcpy(&data.myData, &someData[0], someData.size());
	data.myLength = someData.size();
	data.myAddress = aTargetAddress;

	ReadWriteLock lock(mySendAndReceiveMutex);
	myOutgoingNetworkDataBuffer.push_back(data);
#else
	int result = sendto(mySocket, &someData[0], someData.size(), 0, (struct sockaddr*)&aTargetAddress, sizeof(aTargetAddress));
	if (result == SOCKET_ERROR)
	{
		WSACleanup();
		assert(true && "Failed to send message!");
	}
#endif
}

#if THREAD_MODE
void Network::SendAndReceiveThreadFunction()
{
	int bytesReceived = 0;
	int size = sizeof(sockaddr_in);

	const int bufferSize = 512;
	char buffer[bufferSize];

	NetworkData dataToAdd;

	while (myIsRunning)
	{
		{
			// We only need to lock the mutex while we are manipulating the ActiveLists and Buffers
			ReadWriteLock lock(mySendAndReceiveMutex);

			if (myOutgoingNetworkDataBuffer.size() > 0)
			{
				// Copy over all the messages that we want to send to the Active-list
				for (const NetworkData& data : myOutgoingNetworkDataBuffer)
					myActiveOutgoingNetworkData.push_back(data);

				myOutgoingNetworkDataBuffer.clear();
			}

			// If we recieved any new data the last frame, then we need to sync that to the Buffer that is available to the rest of the program
			if (myActiveIncommingNetworkData.size() > 0)
			{
				for (const NetworkData& data : myActiveIncommingNetworkData)
					myIncommingNetworkDataBuffer.push_back(data);

				myActiveIncommingNetworkData.clear();
			}
		}



		// Send all the messages from the Active-list
		for (const NetworkData& data : myActiveOutgoingNetworkData)
		{
			int result = sendto(mySocket, data.myData, data.myLength, 0, (struct sockaddr*)&data.myAddress, sizeof(data.myAddress));
			if (result == SOCKET_ERROR)
			{
				WSACleanup();
				assert(true && "Failed to send message!");
				myIsRunning = false;
			}
		}

		myActiveOutgoingNetworkData.clear();


		ZeroMemory(&buffer, bufferSize);

		// Read from network and populate the ActiveIncommingNetworkData
		// Doing so doesnt require a mutexlock since that array is used exclusively by this thread
		// Any messages that are received will be copied to the Buffer at the start of the next update
		while ((bytesReceived = recvfrom(mySocket, buffer, bufferSize, 0, (struct sockaddr *)&dataToAdd.myAddress, &size)) > 0)
		{
			memcpy(&dataToAdd.myData, &buffer[0], bytesReceived * sizeof(char));
			dataToAdd.myLength = bytesReceived;
			myActiveIncommingNetworkData.push_back(dataToAdd);
		}
	}
}

#endif