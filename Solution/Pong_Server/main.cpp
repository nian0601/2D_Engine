#include "SFML_Engine.h"
#include "Pong_Server.h"

int main(int, char**)
{
	SFML_Engine engine(640, 320, "Pong Server");
	Pong_Server pong;

	engine.Run(pong);

	return 0;
}