#pragma once
#include "FW_Includes.h"
#include "FW_IGame.h"

#include "Components.h"
#include "Messages.h"

class FW_EntityManager;
class Breakout : public FW_IGame
{
public:
	void OnStartup() override;
	bool Run() override;
	void RunEditorLogic() override;
	const char* GetGameName() override { return "Breakout"; }
	const char* GetDataFolderName() override { return "Breakout"; }

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

	int myDamageSoundEffectID = -1;

	FW_EntityManager* myEntityManager;
	Rectf myGameArea;

	Vector2f myGridSize;
	Vector2f myMouseDownPosition;
	Rectf mySelectedGridArea;
	FW_GrowingArray<Rectf> myEditorGrid;
	bool myShowGrid = false;
	bool myIsInEditorMode = false;
	bool myDebugDrawCollision = false;
	bool myDebugDrawGameArea = false;
	bool myShowDemoWindow = false;
};