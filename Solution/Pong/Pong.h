#pragma once

#include "Paddle.h"
#include "Ball.h"
#include "Score.h"

#include "FW_IGame.h"

class Pong : public FW_IGame
{
public:
	Pong();

	bool Run() override;

private:
	enum GameState
	{
		NEW_ROUND,
		RUNNING,
		SOMEONE_SCORED,
		GAME_OVER
	};

	void UpdateNewRound();
	void UpdateRunning(float aDelta);
	void UpdateSomeoneScored();
	void UpdateGameOver();

	void StartNewRound();

	Paddle myLeftPlayer;
	Paddle myRightPlayer;
	Ball myBall;
	Score myScore;

	GameState myGameState;
};
