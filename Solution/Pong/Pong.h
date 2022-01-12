#pragma once

#include "Paddle.h"
#include "Ball.h"
#include "Score.h"

#include "FW_IGame.h"

class Pong : public FW_IGame
{
public:
	void OnStartup() override;
	void OnShutdown() override;
	bool Run() override;

	const char* GetGameName() override { return "Pong"; }
	const char* GetDataFolderName() override { return "Pong"; }

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

	Paddle* myLeftPlayer;
	Paddle* myRightPlayer;
	Ball* myBall;
	Score* myScore;

	GameState myGameState;
};
