#pragma once

#include "FW_Vector2.h"
#include "FW_Rect.h"
#include "FW_GrowingArray.h"

namespace FW_Renderer
{
	struct Texture
	{
		Vector2i mySize;
		int myTextureID = -1;
	};

	struct Camera
	{
		Vector2f myPosition;
	};

	void Clear();
	void FinishOffscreenBuffer();
	void ResizeOffscreenBuffer(int aWidth, float anAspectRatio);
	void ResizeOffscreenBuffer(int aWidth, int aHeight);
	void Present();

	void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor = 0xFFFFFFFF);
	void RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor = 0xFFFFFFFF);
	void RenderRect(const Rectf& aRect, int aColor = 0xFFFFFFFF, float aRotationInRadians = 0.f);
	void RenderCircle(const Vector2f& aCenter, float aRadius, int aColor = 0xFFFFFFFF);
	void RenderTexture(const Texture& aTexture, const Vector2i& aPos, float aRotationInRadians = 0.f);
	void RenderTexture(const Texture& aTexture, const Vector2i& aPos, const Recti& aTextureRect, float aRotationInRadians = 0.f);
	void RenderTexture(int aTextureID, const Vector2i& aPos, const Vector2i& aSize, const Recti& aTextureRect, float aRotationInRadians = 0.f);
	void RenderTexture(int aTextureID, const Recti& aSpriteRect, const Recti& aTextureRect, float aRotationInRadians = 0.f);

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
	int GetOffscreenBufferWidth();
	int GetOffscreenBufferHeight();
	Texture CreateTexture(const Vector2i& aSize);
	void DeleteTexture(const Texture& aTexture);
	void UpdatePixelsInTexture(const Texture& aTexture, void* somePixels);
	void SaveTextureToFile(const Texture& aTexture, const char* aFileName);

	Camera& GetCamera();
	const Vector2f& GetCameraPosition();
}