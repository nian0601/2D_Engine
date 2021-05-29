#pragma once

class FW_EntityManager;
struct FW_CollisionMessage;

namespace BallSystem
{
	void OnCollision(FW_EntityManager& aEntityManager, const FW_CollisionMessage& aMessage);
};