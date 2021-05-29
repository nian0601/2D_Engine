#include "FW_CollisionSystem.h"

#include "FW_EntityManager.h"
#include "FW_MessageQueue.h"
#include "FW_Components.h"
#include "FW_Messages.h"

namespace FW_CollisionSystem
{
	bool CheckFilter(const CollisionComponent& a, const CollisionComponent& b)
	{
		return (a.myCollidesWithFlags & b.myCollisionFlags) > 0;
	}

	int CheckCollision(const Rectf& a, const Rectf& b)
	{
		if (a.myBottomRight.x < b.myTopLeft.x) return FW_CollisionDirection::LEFTSIDE;
		if (a.myTopLeft.x > b.myBottomRight.x) return FW_CollisionDirection::RIGHTSIDE;

		if (a.myTopLeft.y > b.myBottomRight.y) return FW_CollisionDirection::TOPSIDE;
		if (a.myBottomRight.y < b.myTopLeft.y) return FW_CollisionDirection::BOTTOMSIDE;

		return FW_CollisionDirection::COLLISION;
	}

	int CheckCollision(FW_MessageQueue& aMessageQueue, const CollisionComponent& a, const CollisionComponent& b)
	{
		int prevState = CheckCollision(a.myPreviousRect, b.myRect);
		int currState = CheckCollision(a.myRect, b.myRect);
		if (currState == FW_CollisionDirection::COLLISION)
		{
			aMessageQueue.QueueMessage<FW_CollisionMessage>({ a.myEntity, b.myEntity, FW_CollisionDirection(prevState) });
			return prevState;
		}

		return FW_CollisionDirection::CLEAR;
	}

	void HandleCollision(FW_EntityManager& aEntityManger, int aCollisionSide, CollisionComponent& aCollision)
	{
		if (aCollisionSide == FW_CollisionDirection::CLEAR)
			return;

		if (TranslationComponent* translation = aEntityManger.FindComponent<TranslationComponent>(aCollision.myEntity))
		{
			switch (aCollisionSide)
			{
			case FW_CollisionDirection::LEFTSIDE:
			case FW_CollisionDirection::RIGHTSIDE:
				translation->myPosition.x = aCollision.myPreviousRect.myCenterPos.x;
				break;
			case FW_CollisionDirection::TOPSIDE:
			case FW_CollisionDirection::BOTTOMSIDE:
				translation->myPosition.y = aCollision.myPreviousRect.myCenterPos.y;
				break;
			}

			SetRectPosition(aCollision.myRect, translation->myPosition);
		}
	}


	void Run(FW_EntityManager& aEntityManger)
	{
		FW_ComponentStorage<CollisionComponent>& collisionStorage = aEntityManger.GetComponentStorage<CollisionComponent>();

		for (CollisionComponent& collision : collisionStorage)
		{
			if (const TranslationComponent* translation = aEntityManger.FindComponent<TranslationComponent>(collision.myEntity))
			{
				collision.myPreviousRect = collision.myRect;
				SetRectPosition(collision.myRect, translation->myPosition);
			}
		}

		int count = collisionStorage.Count();
		for (int i = 0; i < count - 1; ++i)
		{
			CollisionComponent& collisionA = collisionStorage[i];
			for (int j = i + 1; j < count; ++j)
			{
				CollisionComponent& collisionB = collisionStorage[j];

				bool aCanHitB = CheckFilter(collisionA, collisionB);
				bool bCanHitA = CheckFilter(collisionB, collisionA);

				if (!aCanHitB && !bCanHitA)
					continue;

				int collisionFlagsA = FW_CollisionDirection::CLEAR;
				int collisionFlagsB = FW_CollisionDirection::CLEAR;
				if (aCanHitB)
					collisionFlagsA = CheckCollision(aEntityManger.GetMessageQueue(), collisionA, collisionB);

				if (bCanHitA)
					collisionFlagsB = CheckCollision(aEntityManger.GetMessageQueue(), collisionB, collisionA);

				if (collisionFlagsA != FW_CollisionDirection::CLEAR)
					HandleCollision(aEntityManger, collisionFlagsA, collisionA);

				if (collisionFlagsB != FW_CollisionDirection::CLEAR)
					HandleCollision(aEntityManger, collisionFlagsB, collisionB);
			}
		}
	}

	void DebugDraw(FW_EntityManager& aEntityManger)
	{
		FW_ComponentStorage<CollisionComponent>& collisionStorage = aEntityManger.GetComponentStorage<CollisionComponent>();

		for (CollisionComponent& collision : collisionStorage)
			FW_Renderer::RenderRect(collision.myRect, 0xAA999999);
	}

}

