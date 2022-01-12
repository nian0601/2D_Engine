#include "stdafx.h"
#include "SFML_Engine.h"
#include "Breakout.h"

int main(int, char**)
{
	SFML_Engine engine(1280, 800);
	Breakout game;

	engine.Run(game);

	return 0;
}