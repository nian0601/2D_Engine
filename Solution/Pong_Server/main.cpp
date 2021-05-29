#include "SFML_Engine.h"
#include "Pong_Server.h"

int main(int, char**)
{
	SFML_Engine engine(1280, 720, "Pong Server");
	Pong_Server pong;

	engine.Run(pong);

	return 0;
}