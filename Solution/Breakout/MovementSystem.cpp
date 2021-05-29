#include "stdafx.h"
#include "MovementSystem.h"
#include "FW_EntityManager.h"
#include "FW_Time.h"

namespace MovementSystem
{
	void Run(FW_EntityManager& aEntityManager)
	{
		float delta = FW_Time::GetDelta();

		FW_ComponentStorage<MovementComponent>& movementStorage = aEntityManager.GetComponentStorage<MovementComponent>();
		for (MovementComponent& movement : movementStorage)
		{
			if (TranslationComponent* translation = aEntityManager.FindComponent<TranslationComponent>(movement.myEntity))
				translation->myPosition += movement.myDirection * movement.mySpeed * delta;
		}
	}
}