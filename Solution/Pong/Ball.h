#pragma once

class Paddle;
class Ball
{
public:
	Ball();

	void Update(float aDelta);
	void Render();

	// Returns true if the ball has passed this paddle
	bool HasPassed(const Paddle& aPaddle);

	void ResetPosition();
	void BounceX() { myVX *= -1.f; }


	int Width() const { return myTexture.mySize.x; }
	int Height() const { return myTexture.mySize.y; }
	float GetX() const { return myX; }
	float GetY() const { return myY; }

	float GetVX() const { return myVX; }
	float GetVY() const { return myVY; }

private:

	FW_Renderer::Texture myTexture;
	float myX;
	float myY;

	float myVX;
	float myVY;
};
