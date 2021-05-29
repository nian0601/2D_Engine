#include "stdafx.h"

#include "Paddle.h"
#include "Ball.h"

Paddle::Paddle(bool aIsPlayer)
	: myIsPlayer(aIsPlayer)
{
	myTexture = FW_Renderer::GetTexture("pong_paddle.png");

	ResetPosition();

	if (myIsPlayer)
		myX = 100.f;
	else
		myX = FW_Renderer::GetScreenWidth() - 100.f;


	myIsLeftPaddle = myX < FW_Renderer::GetScreenWidth() * 0.5f;
}

void Paddle::Update(float aDelta)
{
	if (myIsPlayer)
		UpdatePlayer(aDelta);
	else
		UpdateAI(aDelta);

	if (myY <= 0.f)
		myY = 0.f;
	else if (myY >= FW_Renderer::GetScreenHeight() - myTexture.mySize.y)
		myY = static_cast<float>(FW_Renderer::GetScreenHeight() - myTexture.mySize.y);
}

void Paddle::Render()
{
	// Allow floats in the interface and then round them to int inside?
	FW_Renderer::RenderTexture(myTexture, { static_cast<int>(myX), static_cast<int>(myY) });
}

bool Paddle::CheckCollision(const Ball& aBall) const
{
	// The ball wont collide with us if its moving away
	if (myIsLeftPaddle && aBall.GetVX() > 0.f) return false;
	if (!myIsLeftPaddle && aBall.GetVX() < 0.f) return false;

	// Is Above us
	if (aBall.GetY() + aBall.Height() < myY) return false;

	// Below us
	if (aBall.GetY() > myY + Height()) return false;

	// Passed our Right side
	if (aBall.GetX() > myX + Width()) return false;

	// Passed our Left side
	if (aBall.GetX() + aBall.Width() < myX) return false;
	
	return true;
}

void Paddle::ResetPosition()
{
	myY = FW_Renderer::GetScreenHeight() * 0.5f;
	myY -= myTexture.mySize.y * 0.5f;
}

void Paddle::UpdatePlayer(float aDelta)
{
	const float speed = 10.f;
	if (FW_Input::IsKeyDown(FW_Input::KeyCode::W))
		myY -= speed * aDelta;
	else if (FW_Input::IsKeyDown(FW_Input::KeyCode::S))
		myY += speed * aDelta;
}

void Paddle::UpdateAI(float aDelta)
{
	const float speed = 10.f;
	if (FW_Input::IsKeyDown(FW_Input::KeyCode::UP))
		myY -= speed * aDelta;
	else if (FW_Input::IsKeyDown(FW_Input::KeyCode::DOWN))
		myY += speed * aDelta;
}
