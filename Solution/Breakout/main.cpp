#include "stdafx.h"
#include "SFML_Engine.h"
#include "Game.h"

int main(int, char**)
{
	SFML_Engine engine(1280, 800);
	Game game;

	engine.Run(game);

	return 0;
}