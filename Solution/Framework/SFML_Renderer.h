#pragma once

namespace sf
{
	class RenderWindow;
	class Texture;
}

namespace SFML_Renderer
{
	void Init(sf::RenderWindow* aRenderWindow);
	void ResizeOffscreenBuffer(int aWidth, float anAspectRatio);
	void Shutdown();
	void SetDataFolder(const char* aFolderName);

	const sf::Texture& GetOffscreenBuffer();
}
