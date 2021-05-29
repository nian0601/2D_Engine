#include "stdafx.h"

#include "Pong.h"

Pong::Pong()
	: myLeftPlayer(true)
	, myRightPlayer(false)
	, myGameState(NEW_ROUND)
{
}

bool Pong::Run()
{
	float delta = 1.f / 60.f;
	switch (myGameState)
	{
	case Pong::NEW_ROUND:
		UpdateNewRound();
		break;
	case Pong::RUNNING:
		UpdateRunning(delta);
		break;
	case Pong::SOMEONE_SCORED:
		UpdateSomeoneScored();
		break;
	case Pong::GAME_OVER:
		UpdateGameOver();
		break;
	default:
		break;
	}

	myLeftPlayer.Render();
	myRightPlayer.Render();
	myBall.Render();
	myScore.Render();
	return true;
}

void Pong::UpdateNewRound()
{
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::SPACE))
		myGameState = RUNNING;
}

void Pong::UpdateRunning(float aDelta)
{
	myLeftPlayer.Update(aDelta);
	myRightPlayer.Update(aDelta);
	myBall.Update(aDelta);

	if (myRightPlayer.CheckCollision(myBall) || myLeftPlayer.CheckCollision(myBall))
		myBall.BounceX();

	bool leftPlayerScored = myBall.HasPassed(myRightPlayer);
	bool rightPlayerScored = myBall.HasPassed(myLeftPlayer);
	if (leftPlayerScored || rightPlayerScored)
	{
		if (leftPlayerScored)
			myScore.AddLeftPlayerScore();
		else
			myScore.AddRightPlayerScore();

		myGameState = SOMEONE_SCORED;
	}
}

void Pong::UpdateSomeoneScored()
{
	if (myScore.GameIsOver())
	{
		myGameState = GAME_OVER;
	}
	else
	{
		StartNewRound();
	}
}

void Pong::UpdateGameOver()
{
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::SPACE))
	{
		myScore.Reset();
		StartNewRound();
	}
}

void Pong::StartNewRound()
{
	myLeftPlayer.ResetPosition();
	myRightPlayer.ResetPosition();
	myBall.ResetPosition();
	myGameState = NEW_ROUND;
}
