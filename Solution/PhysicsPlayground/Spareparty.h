#pragma once

#include "PhysicsWorld.h"

#include <FW_Vector2.h>
#include <FW_EntityManager.h>
#include <FW_FileSystem.h>
#include <FW_Messages.h>

class Spareparty
{
public:
	Spareparty();
	~Spareparty();

	void Run();

private:
	void CreateTile(const Vector2f& aPosition, int aTileID);
	void CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName = "");
	void CreatePlayer(const Vector2f& aPosition);

	FW_EntityID GetEntityUnderMouse();
	Vector2f SnapPositionToGrid(const Vector2f& aPosition) const;

	void OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage);

	PhysicsWorld myPhysicsWorld;
	FW_EntityManager myEntityManager;

	FW_GrowingArray<FW_Renderer::Texture> myTileTextures;
	FW_Renderer::Texture mySelectedTexture;
	bool myRenderPhysicsObjects = false;
};
