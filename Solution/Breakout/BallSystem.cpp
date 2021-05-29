#include "stdafx.h"

#include "BallSystem.h"
#include "FW_EntityManager.h"
	
namespace BallSystem
{
	void OnCollision(FW_EntityManager& aEntityManager, const FW_CollisionMessage& aMessage)
	{
		if (!aEntityManager.FindComponent<BallComponent>(aMessage.myEntity))
			return;

		if (aEntityManager.FindComponent<KillBallComponent>(aMessage.myEntityThatHitMe))
		{
			aEntityManager.QueueEntityRemoval(aMessage.myEntity);
			return;
		}

		if (MovementComponent* movement = aEntityManager.FindComponent<MovementComponent>(aMessage.myEntity))
		{
			switch (aMessage.myCollisionDirection)
			{
			case FW_CollisionDirection::LEFTSIDE:
			case FW_CollisionDirection::RIGHTSIDE:
				movement->myDirection.x *= -1.f;
				break;
			case FW_CollisionDirection::TOPSIDE:
			case FW_CollisionDirection::BOTTOMSIDE:
				movement->myDirection.y *= -1.f;
				break;
			}
		}
	}
}
