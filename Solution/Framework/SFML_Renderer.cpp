#include "FW_Renderer.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <assert.h>
#include <windows.h>
#include <sstream>

namespace SFML_Renderer
{
	static sf::RenderWindow* ourRenderWindow = nullptr;
	static sf::Font* ourFont;
	static sf::RectangleShape ourRectangleShape;
	static Vector2f ourCurrentRectangleSize;
	static sf::IntRect ourCurrentTextureRect;

	struct CachedTexture
	{
		sf::Texture mySFMLTexture;
		std::string myFileName;
		FW_Renderer::Texture myRendererTexture;
	};
	static int ourFreeTextureID = 0;
	static std::vector<CachedTexture> ourCachedTextures;

	static std::string ourAssetPath;

	const sf::Texture* GetSFMLTexture(int aTextureID)
	{
		for (const CachedTexture& cachedTexture : ourCachedTextures)
		{
			if (cachedTexture.myRendererTexture.myTextureID == aTextureID)
				return &cachedTexture.mySFMLTexture;
		}

		assert(false && "Failed to find cached SDL_Texture");
		return nullptr;
	}

	void ReplaceAllOccurancesInString(std::string& aString, const std::string& aSomethingToReplace, const std::string& aNewString)
	{
		// Get the first occurrence
		size_t pos = aString.find(aSomethingToReplace);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			aString.replace(pos, aSomethingToReplace.size(), aNewString);
			// Get the next occurrence from the current position
			pos = aString.find(aSomethingToReplace, pos + aNewString.size());
		}
	}

	void Init(sf::RenderWindow* aRenderWindow)
	{
		ourRenderWindow = aRenderWindow;

		char fileBuffer[128];
		assert(GetModuleFileNameA(NULL, fileBuffer, 128) != 0 && "Failed to get Executable path");
		ourAssetPath.append(fileBuffer);
		ourAssetPath.erase(ourAssetPath.rfind("\\"), std::string::npos);
		ReplaceAllOccurancesInString(ourAssetPath, "\\", "/");
		ourAssetPath.append("/Data/");

		std::string fontPath = ourAssetPath;
		fontPath.append("OpenSans-Regular.ttf");

		ourFont = new sf::Font();
		if (!ourFont->loadFromFile(fontPath))
			assert(false && "Failed to load font");
	}

	void Shutdown()
	{
		ourCachedTextures.clear();
		delete ourFont;
	}

	sf::Color GetSFMLColor(int aHexColor)
	{
		return{
			unsigned char((aHexColor >> 16) & 255),
			unsigned char((aHexColor >> 8) & 255),
			unsigned char((aHexColor >> 0) & 255),
			unsigned char((aHexColor >> 24) & 255)
		};
	}
}

namespace FW_Renderer
{
	void Clear()
	{
		SFML_Renderer::ourRenderWindow->clear();
	}

	void Present()
	{
		SFML_Renderer::ourRenderWindow->display();
	}

	void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor)
	{
		sf::VertexArray line(sf::Lines, 2);
		line[0].position = { float(aStart.x), float(aStart.y) };
		line[0].color = SFML_Renderer::GetSFMLColor(aColor);

		line[1].position = { float(aEnd.x), float(aEnd.y) };
		line[1].color = SFML_Renderer::GetSFMLColor(aColor);
;
		SFML_Renderer::ourRenderWindow->draw(line);
	}

	void RenderRect(const Rectf& aRect, int aColor)
	{
		sf::RectangleShape& rect = SFML_Renderer::ourRectangleShape;
		rect.setPosition({ float(aRect.myTopLeft.x), float(aRect.myTopLeft.y) });

		if (aRect.myExtents != SFML_Renderer::ourCurrentRectangleSize)
		{
			SFML_Renderer::ourCurrentRectangleSize = aRect.myExtents;
			rect.setSize({ aRect.myExtents.x, aRect.myExtents.y });
		}

		rect.setFillColor(SFML_Renderer::GetSFMLColor(aColor));
		rect.setTexture(nullptr);

		SFML_Renderer::ourRenderWindow->draw(rect);
	}

	void RenderTexture(const Texture& aTexture, const Vector2i& aPos)
	{
		if (const sf::Texture* texture = SFML_Renderer::GetSFMLTexture(aTexture.myTextureID))
		{
			sf::Sprite sprite;
			sprite.setTexture(*texture);
			sprite.setPosition({ float(aPos.x), float(aPos.y) });
			SFML_Renderer::ourRenderWindow->draw(sprite);
		}
	}

	void RenderTexture(const Texture& aTexture, const Vector2i& aPos, const Recti& aTextureRect)
	{
		if (const sf::Texture* texture = SFML_Renderer::GetSFMLTexture(aTexture.myTextureID))
		{
			sf::Sprite sprite;
			sprite.setTexture(*texture);
			sprite.setPosition({ float(aPos.x), float(aPos.y) });

			sf::IntRect textRect;
			textRect.left = aTextureRect.myTopLeft.x;
			textRect.top = aTextureRect.myTopLeft.y;
			textRect.width = aTextureRect.myExtents.x;
			textRect.height = aTextureRect.myExtents.y;

			
			sprite.setTextureRect(textRect);
			SFML_Renderer::ourRenderWindow->draw(sprite);
		}
	}
	void RenderTexture(int aTextureID, const Vector2i& aPos, const Vector2i& aSize, const Recti& aTextureRect)
	{
		Recti spriteRect = MakeRect(aPos, aSize);
		RenderTexture(aTextureID, spriteRect, aTextureRect);
	}
	void RenderTexture(int aTextureID, const Recti& aSpriteRect, const Recti& aTextureRect)
	{
		if (const sf::Texture* texture = SFML_Renderer::GetSFMLTexture(aTextureID))
		{
			sf::RectangleShape& rect = SFML_Renderer::ourRectangleShape;
			rect.setFillColor(SFML_Renderer::GetSFMLColor(0xFFFFFFFF));
			rect.setPosition({ float(aSpriteRect.myCenterPos.x), float(aSpriteRect.myCenterPos.y) });

			if (aSpriteRect.myExtents.x != SFML_Renderer::ourCurrentRectangleSize.x && aSpriteRect.myExtents.y != SFML_Renderer::ourCurrentRectangleSize.y)
			{
				SFML_Renderer::ourCurrentRectangleSize.x = static_cast<float>(aSpriteRect.myExtents.x);
				SFML_Renderer::ourCurrentRectangleSize.y = static_cast<float>(aSpriteRect.myExtents.y);
				rect.setSize({ SFML_Renderer::ourCurrentRectangleSize.x, SFML_Renderer::ourCurrentRectangleSize.y });
			}

			rect.setTexture(texture);

			sf::IntRect textRect;
			textRect.left = aTextureRect.myTopLeft.x;
			textRect.top = aTextureRect.myTopLeft.y;
			textRect.width = aTextureRect.myExtents.x;
			textRect.height = aTextureRect.myExtents.y;

			if (textRect != SFML_Renderer::ourCurrentTextureRect)
			{
				SFML_Renderer::ourCurrentTextureRect = textRect;
				rect.setTextureRect(textRect);
			}
			SFML_Renderer::ourRenderWindow->draw(rect);
		}
	}

	void RenderText(const char* aString, const Vector2i& aPos, int aColor, TextAlignment aTextAlignment)
	{
		sf::Text text;
		text.setFont(*SFML_Renderer::ourFont);
		text.setString(aString);
		text.setFillColor(SFML_Renderer::GetSFMLColor(aColor));

		Vector2i pos = aPos;
		if (aTextAlignment == TextAlignment::CENTER)
		{
			sf::FloatRect bounds = text.getGlobalBounds();
			pos.x -= static_cast<int>(bounds.width * 0.5f);
		}
		else if (aTextAlignment == TextAlignment::RIGHT)
		{
			sf::FloatRect bounds = text.getGlobalBounds();
			pos.x -= static_cast<int>(bounds.width);
		}

		text.setPosition({ float(pos.x) , float(pos.y) });
		SFML_Renderer::ourRenderWindow->draw(text);
	}
	void RenderFloat(float aFloat, const Vector2i& aPos, int aColor)
	{
		std::stringstream ss;
		ss << aFloat;
		RenderText(ss.str().c_str(), aPos, aColor);
	}

	Texture GetTexture(const char* aFilePath)
	{
		for (const SFML_Renderer::CachedTexture& cachedTexture : SFML_Renderer::ourCachedTextures)
		{
			if (cachedTexture.myFileName == aFilePath)
				return cachedTexture.myRendererTexture;
		}

		std::string fullPath = SFML_Renderer::ourAssetPath;
		fullPath.append(aFilePath);

		sf::Texture texture;
		if (!texture.loadFromFile(fullPath))
			assert(false && "Failed to load texture.");

		sf::Vector2u size = texture.getSize();

		Texture result;
		result.myTextureID = SFML_Renderer::ourFreeTextureID++;
		result.mySize.x = size.x;
		result.mySize.y = size.y;

		SFML_Renderer::CachedTexture cachedTexture;
		cachedTexture.mySFMLTexture = texture;
		cachedTexture.myRendererTexture = result;
		cachedTexture.myFileName = aFilePath;
		SFML_Renderer::ourCachedTextures.push_back(cachedTexture);

		return result;
	}

	int GetScreenWidth()
	{
		sf::Vector2u size = SFML_Renderer::ourRenderWindow->getSize();
		return size.x;
	}
	int GetScreenHeight()
	{
		sf::Vector2u size = SFML_Renderer::ourRenderWindow->getSize();
		return size.y;
	}
}