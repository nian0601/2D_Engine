#pragma once

namespace sf
{
	class RenderWindow;
	class Texture;
}

namespace SFML_Renderer
{
	void Init(sf::RenderWindow* aRenderWindow);
	void Shutdown();
	void SetDataFolder(const char* aFolderName);

	const sf::Texture& GetOffscreenBuffer();
}
