#pragma once

#include <winsock2.h>
#include "NetworkMessage.h"
#include "Mutex.h"

#pragma comment(lib, "Ws2_32.lib")

#define THREAD_MODE 1

struct NetworkData
{
	char myData[512];
	int myLength;
	sockaddr_in myAddress;
};

namespace std
{
	class thread;
}

class Network
{
public:
	Network();
	~Network();

	void Start();

	bool RecieveMessages(std::vector<NetworkData>& someBuffers);

protected:
	virtual void OnStart() = 0;

	SOCKET mySocket;

	// Data that will be consumed by the Send-Thread and sent over the network
	std::vector<NetworkData> myActiveOutgoingNetworkData;
	// Data that is getting filled by the main program, will be synced over to the 'Active'-list at the start of each SendThread-execution
	std::vector<NetworkData> myOutgoingNetworkDataBuffer;
	Mutex myOutgoingMutex;

	// Will be fed with data that is getting read from the network in the RecieveThread, gets copied over to the 'Buffer'-list at the end of each RecieveThread-execution
	std::vector<NetworkData> myActiveIncommingNetworkData;
	// Contains all the latest incomming networkdata to be consumed by the main program
	std::vector<NetworkData> myIncommingNetworkDataBuffer;
	Mutex myIncommingMutex;

private:
#if THREAD_MODE
	void RecieveThreadFunction();
	void SendThreadFunction();

	volatile bool myIsRunning;
	std::thread* myRecieveThread;
	std::thread* mySendThread;
#endif
};
