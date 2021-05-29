#pragma once

struct FW_CollisionMessage;
struct PowerUpComponent;

class FW_EntityManager;

namespace PowerupSystem
{
	void OnEntityRemoved(FW_EntityManager& aEntityManager, FW_EntityID anEntityID);
	void OnCollision(FW_EntityManager& aEntityManager, const FW_CollisionMessage& aMessage);
};