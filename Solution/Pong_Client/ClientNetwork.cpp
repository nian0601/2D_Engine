#include "ClientNetwork.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

void ClientNetwork::OnStart()
{
	WSADATA WSAData;
	int result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (result != 0)
	{
		WSACleanup();
		assert(true && "WSA Startup failed");
	}

	mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mySocket == INVALID_SOCKET)
	{
		WSACleanup();
		assert(true && "Failed to create socket");
	}

	DWORD nonBlocking = 1;
	if (ioctlsocket(mySocket, FIONBIO, &nonBlocking) != 0)
	{
		WSACleanup();
		assert(true && "Failed to setup NonBlocking socket");
	}

	ZeroMemory(&myServerAddress, sizeof(myServerAddress));
	myServerAddress.sin_family = AF_INET;
	myServerAddress.sin_port = htons(u_short(44556));
	inet_pton(AF_INET, "127.0.0.1", &myServerAddress.sin_addr);
}