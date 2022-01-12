#include "SFML_Engine.h"
#include "Pong_Server.h"

int main(int argc, char** argv)
{
	SFML_Engine engine(640, 320, "Pong Server");
	engine.SetWindowPosition(-700, 440);

	Pong_Server pong;

	bool startExtraClients = true;

	if (argc > 1)
	{
		int shouldStartClients = atoi(argv[1]);
		startExtraClients = shouldStartClients == 1;
	}

	if (startExtraClients)
	{
		pong.StartExtraClientProcess(200, 400);
		pong.StartExtraClientProcess(1200, 400);
	}

	engine.Run(pong);

	return 0;
}