#include "ServerNetwork.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

void ServerNetwork::SendNetworkMessage(const char* aMessage, int aSize, const sockaddr_in& aTargetAddress)
{
#if THREAD_MODE
	NetworkData data;
	memcpy(&data.myData, aMessage, aSize);
	data.myLength = aSize;
	data.myAddress = aTargetAddress;

	ReadWriteLock lock(myOutgoingMutex);
	myOutgoingNetworkDataBuffer.push_back(data);
#else
	int result = sendto(mySocket, aMessage, aSize, 0, (struct sockaddr*)&aTargetAddress, sizeof(aTargetAddress));
	if (result == SOCKET_ERROR)
	{
		WSACleanup();
		assert(true && "Failed to send message!");
	}
#endif
}

void ServerNetwork::OnStart()
{
	WSADATA WSAData;
	int result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (result != 0)
	{
		WSACleanup();
		assert(true && "WSA Startup failed");
	}

	addrinfo* addrResult;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Does it matter what number this is?
	std::string portNumber = "445566";

	result = getaddrinfo(nullptr, portNumber.c_str(), &hints, &addrResult);
	if (result != 0)
	{
		freeaddrinfo(addrResult);
		WSACleanup();
		assert(true && "GetAddrInfo failed");
	}

	mySocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (mySocket == INVALID_SOCKET)
	{
		freeaddrinfo(addrResult);
		WSACleanup();
		assert(true && "Failed to create socket");
	}

	result = bind(mySocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));
	if (result == INVALID_SOCKET)
	{
		freeaddrinfo(addrResult);
		WSACleanup();
		assert(true && "Failed to bind socket");
	}

	freeaddrinfo(addrResult);

	DWORD nonBlocking = 1;
	if (ioctlsocket(mySocket, FIONBIO, &nonBlocking) != 0)
	{
		WSACleanup();
		assert(true && "Failed to setup NonBlocking socket");
	}
}