#include "SFML_Engine.h"
#include "Pong_Client.h"

int main(int, char**)
{
	SFML_Engine engine(720, 640, "Pong Client");
	Pong_Client pong;

	engine.Run(pong);

	return 0;
}