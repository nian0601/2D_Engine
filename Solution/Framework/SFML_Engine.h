#pragma once
class FW_IGame;

namespace sf
{
	class RenderWindow;
}
class SFML_Engine
{
public:
	SFML_Engine(int aWidth, int aHeight);
	void Run(FW_IGame& aGame);

private:
	sf::RenderWindow* myRenderWindow;
};
