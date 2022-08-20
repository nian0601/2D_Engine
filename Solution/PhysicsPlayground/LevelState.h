#pragma once

#include <FW_Includes.h>
#include <FW_Messages.h>
#include <FW_StateStack.h>

#include "SparepartyComponents.h"

class FW_EntityManager;
class PhysicsWorld;

class LevelState : public FW_StateStack::State
{
public:
	LevelState(FW_EntityManager& anEntityManager, PhysicsWorld& aPhysicsWorld);

	FW_StateStack::State::UpdateResult OnUpdate() override;

private:
	void OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage);
	void OnCollision(const CollisionMessage& aMessage);

	void LoadTileSheet(const char* aFilePath);
	void LoadTiledLevel(const char* aFilePath);

	void LoadLevel(int aLevelID);
	void LoadLevel(int aMapData[10][10]);

	FW_EntityID CreateTile(const Vector2f& aPosition, int aTileID);
	FW_EntityID CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName = "");
	FW_EntityID CreateGoal(const Vector2f& aPosition);
	FW_EntityID CreatePlayer(const Vector2f& aPosition);

	FW_EntityManager& myEntityManager;
	PhysicsWorld& myPhysicsWorld;

	int myCurrentLevelID;

	struct Tilesheet
	{
		struct TileData
		{
			int myID; // When I have moved over the Hashmap-implementation then this could be used as a key to that for easier lookups
			int myWidth;
			int myHeight;
			FW_String myTexturePath;
		};

		FW_GrowingArray<TileData> myTiles;
	};
	Tilesheet myTileSheet;
};
