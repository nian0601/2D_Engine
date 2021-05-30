#pragma once

#include "FW_Vector2.h"
#include "FW_Rect.h"

namespace FW_Renderer
{
	struct Texture
	{
		Vector2i mySize;
		int myTextureID;
	};

	void Clear();
	void Present();

	void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor = 0xFFFFFFFF);
	void RenderRect(const Rectf& aRect, int aColor = 0xFFFFFFFF);
	void RenderTexture(const Texture& aTexture, const Vector2i& aPos);
	void RenderTexture(const Texture& aTexture, const Vector2i& aPos, const Recti& aTextureRect);
	void RenderTexture(int aTextureID, const Vector2i& aPos, const Vector2i& aSize, const Recti& aTextureRect);
	void RenderTexture(int aTextureID, const Recti& aSpriteRect, const Recti& aTextureRect);

	enum class TextAlignment
	{
		LEFT,
		CENTER,
		RIGHT,
	};
	void RenderText(const char* aString, const Vector2i& aPos, int aColor = 0xFFFFFFFF, TextAlignment aTextAlignment = TextAlignment::LEFT);
	void RenderFloat(float aFloat, const Vector2i& aPos, int aColor = 0xFFFFFFFF);

	Texture GetTexture(const char* aFilePath);
	int GetScreenWidth();
	int GetScreenHeight();
}