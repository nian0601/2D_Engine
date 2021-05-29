#include "stdafx.h"

#include "Ball.h"
#include "Paddle.h"

Ball::Ball()
{
	myTexture = FW_Renderer::GetTexture("pong_ball.png");
	ResetPosition();

	// Needs vector to be properly normalized
	myVX = -0.7f;
	myVY = 0.7f;
}

void Ball::Update(float aDelta)
{
	const float speed = 10.f;
	myX += myVX * speed * aDelta;
	myY += myVY * speed * aDelta;

	if (myY <= 0 && myVY < 0)
		myVY *= -1.f;
	else if (myY >= FW_Renderer::GetScreenHeight() - myTexture.mySize.y)
		myVY *= -1.f;
}

void Ball::Render()
{
	FW_Renderer::RenderTexture(myTexture, { static_cast<int>(myX), static_cast<int>(myY) });
}

bool Ball::HasPassed(const Paddle& aPaddle)
{
	if (aPaddle.IsLeftSide())
	{
		if (myX <= 0)
			return true;
	}
	else
	{
		if (myX + myTexture.mySize.y > FW_Renderer::GetScreenWidth())
			return true;
	}

	return false;
}

void Ball::ResetPosition()
{
	myX = FW_Renderer::GetScreenWidth() * 0.5f;
	myY = FW_Renderer::GetScreenHeight() * 0.5f;

	myX -= myTexture.mySize.x * 0.5f;
	myY -= myTexture.mySize.y  * 0.5f;
}
