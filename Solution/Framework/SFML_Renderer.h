#pragma once

namespace sf
{
	class RenderWindow;
}

namespace SFML_Renderer
{
	void Init(sf::RenderWindow* aRenderWindow);
	void Shutdown();
}
