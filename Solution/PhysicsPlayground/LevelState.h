#pragma once

#include <FW_Includes.h>
#include <FW_Messages.h>
#include <FW_StateStack.h>

#include "SparepartyComponents.h"

class FW_EntityManager;
class FW_XMLParser;

class PhysicsWorld;

class LevelState : public FW_StateStack::State
{
public:
	LevelState(FW_EntityManager& anEntityManager, PhysicsWorld& aPhysicsWorld);

	FW_StateStack::State::UpdateResult OnUpdate() override;

private:
	struct Tilesheet
	{
		struct TileData
		{
			int myID = -1; // When I have moved over the Hashmap-implementation then this could be used as a key to that for easier lookups
			int myWidth = 0;
			int myHeight = 0;
			FW_String myTexturePath;

			bool myIsSpawnPoint = false;
			bool myIsGoal = false;
		};

		FW_GrowingArray<TileData> myTiles;

		FW_String myPlayerTexturePath;
	};

	struct LevelInformation
	{
		Tilesheet myTileSheet;
		Vector2f mySpawnPosition;

		FW_String myLevelName;
		FW_String myNextLevelName;
	};

	void OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage);
	void OnCollision(const CollisionMessage& aMessage);

	void LoadTiledLevel(const char* aLevelName);
	void LoadMapProperties(FW_XMLParser& aParser);
	void LoadMapTileSheets(FW_XMLParser& aParser);
	void LoadMapLayers(FW_XMLParser& aParser);
	void LoadTileSheet(const char* aFilePath, int aFirstTileID);

	FW_EntityID CreateTile(const Vector2f& aPosition, const Tilesheet::TileData& someTileData);
	FW_EntityID CreatePlayer();

	FW_EntityID myPlayerEntity;

	FW_EntityManager& myEntityManager;
	PhysicsWorld& myPhysicsWorld;
	LevelInformation myCurrentLevelInformation;
};
