#pragma once

#include "PhysicsWorld.h"

#include <FW_Vector2.h>
#include <FW_EntityManager.h>
#include <FW_FileSystem.h>
#include <FW_Messages.h>

struct CollisionMessage;

class Spareparty
{
public:
	Spareparty();
	~Spareparty();

	void Run();

private:
	void LoadLevel(int aLevelID);
	void LoadLevel(int aMapData[10][10]);

	FW_EntityID CreateTile(const Vector2f& aPosition, int aTileID);
	FW_EntityID CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName = "");
	FW_EntityID CreateGoal(const Vector2f& aPosition);
	FW_EntityID CreatePlayer(const Vector2f& aPosition);

	FW_EntityID GetEntityUnderMouse();
	Vector2f SnapPositionToGrid(const Vector2f& aPosition) const;

	void OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage);
	void OnCollision(const CollisionMessage& aMessage);

	PhysicsWorld myPhysicsWorld;
	FW_EntityManager myEntityManager;

	FW_GrowingArray<FW_Renderer::Texture> myTileTextures;
	FW_Renderer::Texture mySelectedTexture;
	bool myRenderPhysicsObjects = false;

	int myCurrentLevelID;
};
