#include "stdafx.h"
#include "Breakout.h"

#include "BallSystem.h"
#include "HealthSystem.h"
#include "MovementSystem.h"
#include "PlayerSystem.h"
#include "PowerupSystem.h"

#include "FW_CollisionSystem.h"
#include "FW_Editor.h"
#include "FW_LevelLoader.h"
#include "FW_MessageQueue.h"
#include "FW_RenderSystem.h"
#include "FW_EntityManager.h"
#include "FW_AudioSystem.h"

void Breakout::OnStartup()
{
	SetupEditorGrid();

	myEntityManager = new FW_EntityManager();
	myEntityManager->RegisterComponent<MovementComponent>();
	myEntityManager->RegisterComponent<PlayerComponent>();
	myEntityManager->RegisterComponent<CollisionComponent>();
	myEntityManager->RegisterComponent<HealthComponent>();
	myEntityManager->RegisterComponent<KillBallComponent>();
	myEntityManager->RegisterComponent<BallComponent>();
	myEntityManager->RegisterComponent<PowerUpComponent>();

	FW_MessageQueue& messageQueue = myEntityManager->GetMessageQueue();
	messageQueue.SubscribeToMessage<FW_CollisionMessage>(std::bind(&Breakout::OnCollision, this, std::placeholders::_1));
	messageQueue.SubscribeToMessage<FW_EntityCreatedMessage>(std::bind(&Breakout::OnEntityCreated, this, std::placeholders::_1));
	messageQueue.SubscribeToMessage<FW_PreEntityRemovedMessage>(std::bind(&Breakout::OnPreEntityRemoved, this, std::placeholders::_1));

	StartLevel();
	myTextPosition.x = static_cast<int>(myGameArea.myCenterPos.x);
	myTextPosition.y = static_cast<int>(myGameArea.myBottomRight.y) - 200;

	myDamageSoundEffectID = FW_AudioSystem::LoadClip("UwU.wav");
}

bool Breakout::Run()
{
	FW_RenderSystem::Run(*myEntityManager);

	myIsInEditorMode = FW_Editor::BeginEditor(*myEntityManager);
	if (myIsInEditorMode)
	{
		RunEditorLogic();
		FW_Editor::EndEditor();
	}
	else
	{
		switch (myGameState)
		{
		case GameState::START_SCREEN:
			RunStartScreen();
			break;
		case GameState::WAITING_FOR_NEW_ROUND:
			RunWaitingForNewRound();
			break;
		case GameState::IS_RUNNING:
			RunIsRunning();
			MovementSystem::Run(*myEntityManager);
			break;
		case GameState::GAME_OVER:
			RunGameOver();
			break;
		case GameState::GAME_WON:
			RunGameWon();
			break;
		}

		PlayerSystem::Run(*myEntityManager);
		HealthSystem::Run(*myEntityManager);

		FW_CollisionSystem::Run(*myEntityManager);
	}

	myEntityManager->EndFrame();

	return true;
}

void Breakout::StartLevel()
{
	myEntityManager->QueueRemovalAllEntities();
	myEntityManager->FlushEntityRemovals();

	FW_LevelLoader::LoadLevel(*myEntityManager, "testlevel");

	myGameState = GameState::START_SCREEN;
	myRemainingLives = 3;
}

void Breakout::RunStartScreen()
{
	FW_Renderer::RenderText("--- Press Space to begin ---", myTextPosition, 0xFFFFFFFF, FW_Renderer::TextAlignment::CENTER);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::SPACE))
		myGameState = GameState::IS_RUNNING;
}

void Breakout::RunWaitingForNewRound()
{
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::SPACE))
		myGameState = GameState::IS_RUNNING;
}

void Breakout::RunIsRunning()
{
	FW_ComponentStorage<HealthComponent>& healthStorage = myEntityManager->GetComponentStorage<HealthComponent>();
	if (healthStorage.Count() <= 0)
		myGameState = GameState::GAME_WON;
}

void Breakout::RunGameOver()
{
	FW_Renderer::RenderText("--- GAME OVER LUL ---", myTextPosition, 0xFFFFFFFF, FW_Renderer::TextAlignment::CENTER);
	FW_Renderer::RenderText("Press Space to restart", { myTextPosition.x, myTextPosition.y + 20 }, 0xFFFFFFFF, FW_Renderer::TextAlignment::CENTER);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::SPACE))
		StartLevel();
}

void Breakout::RunGameWon()
{
	FW_Renderer::RenderText("--- YOU WON ---", myTextPosition, 0xFFFFFFFF, FW_Renderer::TextAlignment::CENTER);
	FW_Renderer::RenderText("Press Space to restart", { myTextPosition.x, myTextPosition.y + 20 }, 0xFFFFFFFF, FW_Renderer::TextAlignment::CENTER);
	if (FW_Input::WasKeyReleased(FW_Input::KeyCode::SPACE))
		StartLevel();
}

void Breakout::OnEntityCreated(const FW_EntityCreatedMessage& aMessage)
{
	if (TranslationComponent* translation = myEntityManager->FindComponent<TranslationComponent>(aMessage.myEntity))
	{
		translation->myPosition = aMessage.myPosition;
		translation->mySpawnPosition = aMessage.myPosition;
	}

	if (CollisionComponent* collision = myEntityManager->FindComponent<CollisionComponent>(aMessage.myEntity))
		SetRectPosition(collision->myRect, aMessage.myPosition);

	if (RenderComponent* render = myEntityManager->FindComponent<RenderComponent>(aMessage.myEntity))
		render->myTexture = FW_Renderer::GetTexture("spritesheet.png");
}

void Breakout::OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage)
{
	if (myIsInEditorMode)
		return;

	PowerupSystem::OnEntityRemoved(*myEntityManager, aMessage.myEntity);

	if (myEntityManager->FindComponent<BallComponent>(aMessage.myEntity))
		OnBallRemoved();
}

void Breakout::OnCollision(const FW_CollisionMessage& aMessage)
{
	if (myIsInEditorMode)
		return;

	BallSystem::OnCollision(*myEntityManager, aMessage);
	PowerupSystem::OnCollision(*myEntityManager, aMessage);
	HealthSystem::OnCollision(*myEntityManager, aMessage, myDamageSoundEffectID);
}

void Breakout::OnBallRemoved()
{
	FW_ComponentStorage<BallComponent>& ballStorage = myEntityManager->GetComponentStorage<BallComponent>();
	if (ballStorage.Count() > 1)
		return;

	--myRemainingLives;
	if (myRemainingLives <= 0)
	{
		myGameState = GameState::GAME_OVER;
	}
	else
	{
		Vector2f newBallPosition;
		FW_ComponentStorage<PlayerComponent>& playerStorage = myEntityManager->GetComponentStorage<PlayerComponent>();
		for (const PlayerComponent& player : playerStorage)
		{
			if (TranslationComponent* playerTranslation = myEntityManager->FindComponent<TranslationComponent>(player.myEntity))
			{
				playerTranslation->myPosition = playerTranslation->mySpawnPosition;
				newBallPosition = playerTranslation->myPosition;
				newBallPosition.y -= 25.f;
			}
		}

		FW_LevelLoader::AddEntity(*myEntityManager, newBallPosition, "ball");

		myGameState = GameState::WAITING_FOR_NEW_ROUND;
	}
}

void Breakout::RunEditorLogic()
{
	bool leftMouseDown = FW_Input::WasMousePressed(FW_Input::LEFTMB);
	bool leftMouseUp = FW_Input::WasMouseReleased(FW_Input::LEFTMB);

	if (leftMouseDown)
	{
		myMouseDownPosition = FW_Input::GetMousePositionf();
	}
	else if (leftMouseUp)
	{
		FW_GrowingArray<FW_EntityID> selectedEntities;
		GetEntitiesInSelection(selectedEntities);
		FW_Editor::SetSelectedEntities(selectedEntities);
	}
	else if (FW_Input::IsMouseDown(FW_Input::LEFTMB))
	{
		mySelectedGridArea = MakeRectFromPoints(myMouseDownPosition, FW_Input::GetMousePositionf());
	}

	if (FW_Input::WasKeyPressed(FW_Input::Q))
	{
		FW_Editor::ClearSelectedEntities();
		mySelectedGridArea = Rectf();
	}


	if (FW_Input::WasMousePressed(FW_Input::RIGHTMB))
	{
		FW_GrowingArray<Vector2f> selectedCells;
		for (const Rectf& cell : myEditorGrid)
		{
			if (Collides(cell, mySelectedGridArea))
				selectedCells.Add(cell.myCenterPos);

			FW_Editor::NewEntityPopup(selectedCells);
		}
	}

	for (const Rectf& cell : myEditorGrid)
	{
		int color = 0xAA222222;
		bool mouseIsInCell = Contains(cell, FW_Input::GetMousePositionf());
		bool selectionIsOverCell = Collides(cell, mySelectedGridArea);
		if (selectionIsOverCell)
			color = 0xAAAAAA22;

		if (mouseIsInCell)
			color = 0xAA2222AA;

		if (myShowGrid || mouseIsInCell || selectionIsOverCell)
			FW_Renderer::RenderRect(cell, color);
	}

	if (myDebugDrawCollision)
		FW_CollisionSystem::DebugDraw(*myEntityManager);

	if (myDebugDrawGameArea)
		FW_Renderer::RenderRect(myGameArea, 0x66FFAAFF);

	ImGui::Checkbox("Show Grid", &myShowGrid);
	ImGui::Checkbox("Show Collision", &myDebugDrawCollision);
	ImGui::Checkbox("Show GameArea", &myDebugDrawGameArea);
	ImGui::Separator();
}

void Breakout::SetupEditorGrid()
{
	int numXCells = 15;
	int numYCells = 20;

	Vector2f gridSize = { 32.f, 32.f };
	myGameArea = MakeRect(gridSize.x, gridSize.y, numXCells * gridSize.x, numYCells * gridSize.y);

	Rectf cellRect = MakeRect(0.f, 0.f, gridSize.x - 1.f, gridSize.y - 1.f);
	for (float y = myGameArea.myTopLeft.y; y < myGameArea.myBottomRight.y; y += gridSize.y)
	{
		for (float x = myGameArea.myTopLeft.x; x < myGameArea.myBottomRight.x; x += gridSize.x)
		{
			SetRectPosition(cellRect, { x + gridSize.x * 0.5f, y + gridSize.y * 0.5f });
			myEditorGrid.Add(cellRect);
		}
	}
}

FW_EntityID Breakout::GetEntityUnderMouse()
{
	if (!Contains(myGameArea, FW_Input::GetMousePositionf()))
		return InvalidEntity;

	const FW_ComponentStorage<RenderComponent>& renderStorage = myEntityManager->GetComponentStorage<RenderComponent>();
	for (const RenderComponent& renderComponent : renderStorage)
	{
		if (TranslationComponent* translateComponent = myEntityManager->FindComponent<TranslationComponent>(renderComponent.myEntity))
		{
			Vector2f size(float(renderComponent.mySpriteSize.x), float(renderComponent.mySpriteSize.y));
			Rectf rect = MakeRect(translateComponent->myPosition, size);

			if (Contains(rect, FW_Input::GetMousePositionf()))
				return renderComponent.myEntity;
		}
	}

	return InvalidEntity;
}

void Breakout::GetEntitiesInSelection(FW_GrowingArray<FW_EntityID>& someEntitiesOut)
{
	if (!Contains(myGameArea, FW_Input::GetMousePositionf()))
		return;

	const FW_ComponentStorage<RenderComponent>& renderStorage = myEntityManager->GetComponentStorage<RenderComponent>();
	for (const RenderComponent& renderComponent : renderStorage)
	{
		if (TranslationComponent* translateComponent = myEntityManager->FindComponent<TranslationComponent>(renderComponent.myEntity))
		{
			Vector2f size(float(renderComponent.mySpriteSize.x), float(renderComponent.mySpriteSize.y));
			Rectf rect = MakeRect(translateComponent->myPosition, size);

			if (Collides(rect, mySelectedGridArea))
				someEntitiesOut.Add(renderComponent.myEntity);
		}
	}
}
