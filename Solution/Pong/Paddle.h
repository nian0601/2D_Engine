#pragma once

class Ball;
class Paddle
{
public:
	Paddle(bool aIsPlayer);

	void Update(float aDelta);
	void Render();


	int Width() const { return myTexture.mySize.x; }
	int Height() const { return myTexture.mySize.y; }
	float GetX() const { return myX; }
	float GetY() const { return myY; }
	bool IsLeftSide() const { return myIsLeftPaddle; }

	bool CheckCollision(const Ball& aBall) const;

	void ResetPosition();

private:
	void UpdatePlayer(float aDelta);
	void UpdateAI(float aDelta);

	FW_Renderer::Texture myTexture;
	float myX;
	float myY;
	bool myIsPlayer;
	bool myIsLeftPaddle;
};