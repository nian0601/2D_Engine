#include "Network.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <thread>


Network::Network()
	: myIsRunning(false)
	, myRecieveThread(nullptr)
	, mySendThread(nullptr)
{
}

Network::~Network()
{
	myIsRunning = false;

	myRecieveThread->join();
	delete myRecieveThread;

	mySendThread->join();
	delete mySendThread;

	closesocket(mySocket);
	WSACleanup();
}

void Network::Start()
{
	OnStart();

#if THREAD_MODE
	myIsRunning = true;
	mySendThread = new std::thread([&] {SendThreadFunction(); });
	myRecieveThread = new std::thread([&] {RecieveThreadFunction(); });
#endif
}

bool Network::RecieveMessages(std::vector<NetworkData>& someBuffers)
{
	someBuffers.clear();

#if THREAD_MODE
	ReadWriteLock lock(myIncommingMutex);

	for (const NetworkData& data : myIncommingNetworkDataBuffer)
		someBuffers.push_back(data);

	myIncommingNetworkDataBuffer.clear();

	return !someBuffers.empty();
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
		someBuffers.push_back(dataToAdd);
	}

	return !someBuffers.empty();
#endif
}

#if THREAD_MODE
void Network::RecieveThreadFunction()
{
	int toReturn = 0;
	int size = sizeof(sockaddr_in);

	const int bufferSize = 512;
	char buffer[bufferSize];

	NetworkData dataToAdd;

	while (myIsRunning)
	{
		ZeroMemory(&buffer, bufferSize);

		// Read from network and populate the ActiveIncommingNetworkData
		// Doing so doesnt require a mutexlock since that array is used exclusively by this thread
		while ((toReturn = recvfrom(mySocket, buffer, bufferSize, 0, (struct sockaddr *)&dataToAdd.myAddress, &size)) > 0)
		{
			memcpy(&dataToAdd.myData, &buffer[0], toReturn * sizeof(char));
			dataToAdd.myLength = toReturn;
			myActiveIncommingNetworkData.push_back(dataToAdd);
		}

		// If we recieved any new data, then we need to sync that to the Buffer that is available to the rest of the program,
		// this requires us to lock the mutex to ensure that we're not writing to the Buffer while the main-thread is reading it
		if(myActiveIncommingNetworkData.size() > 0)
		{
			ReadWriteLock lock(myIncommingMutex);
			for (const NetworkData& data : myActiveIncommingNetworkData)
				myIncommingNetworkDataBuffer.push_back(data);

			myActiveIncommingNetworkData.clear();
		}
	}
}

void Network::SendThreadFunction()
{
	while (myIsRunning)
	{
		{
			// Take just a read-lock and check if there is anything in the OutgoingBuffer as a quick
			// test to see if there is anything to process before we block the mutex fully.
			ReadLock lock(myOutgoingMutex);
			if(myOutgoingNetworkDataBuffer.size() == 0)
				continue; //sleep/yield here? Or just keep spinning forever?
		}

		// If we get here then we determined that we have some data to send, so we need to copy that over into the Active-list
		{
			ReadWriteLock lock(myOutgoingMutex);
			for (const NetworkData& data : myOutgoingNetworkDataBuffer)
				myActiveOutgoingNetworkData.push_back(data);

			myOutgoingNetworkDataBuffer.clear();
		}

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
	}
}
#endif