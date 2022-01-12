#include "SFML_Engine.h"
#include "Pong_Client.h"

int main(int argc, char** argv)
{
	SFML_Engine engine(900, 700, "Pong Client");

	if (argc > 1)
	{
		int windowX = atoi(argv[1]);
		int windowY = atoi(argv[2]);

		if(windowX != -1 && windowY != -1)
			engine.SetWindowPosition(windowX, windowY);
	}
	
	Pong_Client pong;
	engine.Run(pong);

	return 0;
}