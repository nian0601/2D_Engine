#include "SFML_Engine.h"
#include "Breakout.h"
#include "Pong.h"
#include "Raytracer.h"

int main(int argc, char** argv)
{
	SFML_Engine engine(1280, 720, "Generic Client");

	//if (argc > 1)
	//{
	//	int windowX = atoi(argv[1]);
	//	int windowY = atoi(argv[2]);
	//
	//	if (windowX != -1 && windowY != -1)
	//		engine.SetWindowPosition(windowX, windowY);
	//}

	//Breakout game;
	//Pong game;
	Raytracer game;
	engine.Run(game);

	return 0;
}