#include "PhysicsSystem.h"

#include <FW_EntityManager.h>
#include <FW_Time.h>
#include <FW_Input.h>
#include "SparepartyComponents.h"
#include "PhysicsWorld.h"

namespace PhysicSystem
{
	void Run(FW_EntityManager& aEntityManager, PhysicsWorld& aPhysicsWorld)
	{
		FW_ComponentStorage<PlayerComponent>& playerStorage = aEntityManager.GetComponentStorage<PlayerComponent>();
		for (const PlayerComponent& player : playerStorage)
		{
			if (PhysicsComponent* playerPhys = aEntityManager.FindComponent<PhysicsComponent>(player.myEntity))
			{
				Vector2f playerForce;
				if (FW_Input::IsKeyDown(FW_Input::A))
					playerForce.x -= 200.f;
				if (FW_Input::IsKeyDown(FW_Input::D))
					playerForce.x += 200.f;
				if (FW_Input::WasKeyReleased(FW_Input::SPACE))
					playerForce.y -= 300000.f;

				playerPhys->myObject->myVelocity += playerPhys->myObject->myInvMass * playerForce;
			}
		}

		aPhysicsWorld.TickLimited(FW_Time::GetDelta());

		FW_ComponentStorage<PhysicsComponent>& physStorage = aEntityManager.GetComponentStorage<PhysicsComponent>();
		for (const PhysicsComponent& physComponent : physStorage)
		{
			if (TranslationComponent* translation = aEntityManager.FindComponent<TranslationComponent>(physComponent.myEntity))
			{
				translation->myPosition = physComponent.myObject->myPosition;
			}			
		}
	}
}
