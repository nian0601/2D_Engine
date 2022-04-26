#pragma once
class FW_IGame;

namespace sf
{
	class RenderWindow;
}
class SFML_Engine
{
public:
	SFML_Engine(int aWidth, int aHeight, const char* aWindowTitle = "SFML Engine");
	void Run(FW_IGame& aGame);

	void SetWindowPosition(int aX, int aY);

private:
	void BuildImGUIStuff();

	sf::RenderWindow* myRenderWindow;
	bool myShowConsole = false;
	bool myShowIMGuiDemoWindow = false;
	char myConsoleInputBuffer[64] = "";
};
