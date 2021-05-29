#pragma once

struct FW_CollisionMessage;

class FW_EntityManager;

namespace HealthSystem
{
	void Run(FW_EntityManager& aEntityManager);
	void OnCollision(FW_EntityManager& aEntityManager, const FW_CollisionMessage& aMessage);
};
