#include "stdafx.h"

#include "PlayerSystem.h"
#include "FW_Input.h"
#include "FW_EntityManager.h"

namespace PlayerSystem
{
	void Run(FW_EntityManager& aEntityManager)
	{
		bool isADown = FW_Input::IsKeyDown(FW_Input::KeyCode::A);
		bool isDDown = FW_Input::IsKeyDown(FW_Input::KeyCode::D);

		bool isWDown = FW_Input::IsKeyDown(FW_Input::KeyCode::W);
		bool isSDown = FW_Input::IsKeyDown(FW_Input::KeyCode::S);

		FW_ComponentStorage<PlayerComponent>& playerStorage = aEntityManager.GetComponentStorage<PlayerComponent>();
		for (PlayerComponent& playerComponent : playerStorage)
		{
			if (MovementComponent* movementComponent = aEntityManager.FindComponent<MovementComponent>(playerComponent.myEntity))
			{
				movementComponent->myDirection.x = 0.f;
				movementComponent->myDirection.y = 0.f;

				if (isADown && isDDown)
					continue;

				if (isADown)
					movementComponent->myDirection.x = -1.f;
				else if (isDDown)
					movementComponent->myDirection.x = 1.f;
				else if (isWDown)
					movementComponent->myDirection.y = -1.f;
				else if (isSDown)
					movementComponent->myDirection.y = 1.f;
			}
		}
	}
}