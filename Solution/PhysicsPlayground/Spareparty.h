#pragma once

#include "PhysicsWorld.h"

#include <FW_Vector2.h>
#include <FW_EntityManager.h>
#include <FW_FileSystem.h>
#include <FW_Messages.h>
#include <FW_StateStack.h>

struct CollisionMessage;

class Spareparty
{
public:
	Spareparty();
	~Spareparty();

	void Run();

private:
	FW_EntityID CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName = "");

	FW_EntityID GetEntityUnderMouse();
	Vector2f SnapPositionToGrid(const Vector2f& aPosition) const;

	PhysicsWorld myPhysicsWorld;
	FW_EntityManager myEntityManager;
	FW_StateStack myStateStack;

	FW_GrowingArray<FW_Renderer::Texture> myTileTextures;
	FW_Renderer::Texture mySelectedTexture;
	bool myRenderPhysicsObjects = false;

	int myCurrentLevelID;
};
