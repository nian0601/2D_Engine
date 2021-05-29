#include "stdafx.h"

#include "SFML_Engine.h"
#include "Pong.h"

int main(int, char**)
{
	SFML_Engine engine(1280, 720);
	Pong pong;

	engine.Run(pong);

	return 0;
}