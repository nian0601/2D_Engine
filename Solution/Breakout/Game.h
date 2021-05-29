#pragma once

#include "FW_IGame.h"

class FW_EntityManager;
class Game : public FW_IGame
{
public:
	Game();
	bool Run() override;
	void RunEditorLogic() override;

private:
	void StartLevel();
	void RunStartScreen();
	void RunWaitingForNewRound();
	void RunIsRunning();
	void RunGameOver();
	void RunGameWon();

	void OnEntityCreated(const FW_EntityCreatedMessage& aMessage);
	void OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage);
	void OnCollision(const FW_CollisionMessage& aMessage);

	void OnBallRemoved();

	void SetupEditorGrid();
	FW_EntityID GetEntityUnderMouse();
	void GetEntitiesInSelection(FW_GrowingArray<FW_EntityID>& someEntitiesOut);

	enum GameState
	{
		START_SCREEN,
		WAITING_FOR_NEW_ROUND,
		IS_RUNNING,
		GAME_OVER,
		GAME_WON,
	};
	GameState myGameState;
	int myRemainingLives;
	Vector2i myTextPosition;

	FW_EntityManager* myEntityManager;
	Rectf myGameArea;

	Vector2f myGridSize;
	Vector2f myMouseDownPosition;
	Rectf mySelectedGridArea;
	FW_GrowingArray<Rectf> myEditorGrid;
	bool myShowGrid = false;
	bool myIsInEditorMode = false;
};