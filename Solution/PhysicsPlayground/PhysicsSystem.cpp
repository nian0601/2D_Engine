#include "PhysicsSystem.h"

#include <FW_EntityManager.h>
#include <FW_Time.h>
#include <FW_Input.h>
#include <FW_Math.h>

#include "SparepartyComponents.h"
#include "PhysicsWorld.h"
#include <FW_MessageQueue.h>
#include <FW_Profiler.h>

namespace PhysicSystem
{
	void Run(FW_EntityManager& aEntityManager, PhysicsWorld& aPhysicsWorld)
	{
		FW_PROFILE_FUNCTION();
		float delta = FW_Time::GetDelta();

		FW_ComponentStorage<PlayerComponent>& playerStorage = aEntityManager.GetComponentStorage<PlayerComponent>();
		for (const PlayerComponent& player : playerStorage)
		{
			if (PhysicsComponent* playerPhys = aEntityManager.FindComponent<PhysicsComponent>(player.myEntity))
			{
				if (FW_Input::IsKeyDown(FW_Input::A))
					playerPhys->myObject->myOrientation -= FW_PI * delta;
				if (FW_Input::IsKeyDown(FW_Input::D))
					playerPhys->myObject->myOrientation += FW_PI * delta;

				if (FW_Input::WasKeyReleased(FW_Input::SPACE))
				{
					FW_Matrix22 orientation;
					orientation.Set(playerPhys->myObject->myOrientation);

					Vector2f up = { 0.f, -1.f };
					up = orientation * up;

					float strenght = 150.f;
					playerPhys->myObject->myVelocity += /*playerPhys->myObject->myInvMass **/ up * strenght;
				}

			}
		}

		aPhysicsWorld.TickLimited(FW_Time::GetDelta());

		FW_ComponentStorage<PhysicsComponent>& physStorage = aEntityManager.GetComponentStorage<PhysicsComponent>();
		for (const PhysicsComponent& physComponent : physStorage)
		{
			if (TranslationComponent* translation = aEntityManager.FindComponent<TranslationComponent>(physComponent.myEntity))
			{
				translation->myPosition = physComponent.myObject->myPosition;
				translation->myOrientation = physComponent.myObject->myOrientation;
			}
		}

		FW_MessageQueue& messageQueue = aEntityManager.GetMessageQueue();
		for (const Manifold& contact : aPhysicsWorld.GetContacts())
		{
			messageQueue.QueueMessage<CollisionMessage>({ contact.myObjectA->myEntityID, contact.myObjectB->myEntityID });
			messageQueue.QueueMessage<CollisionMessage>({ contact.myObjectB->myEntityID, contact.myObjectA->myEntityID });

			//PhysicsComponent* physA = aEntityManager.FindComponent<PhysicsComponent>(contact.myObjectA->myEntityID);
			//PhysicsComponent* physB = aEntityManager.FindComponent<PhysicsComponent>(contact.myObjectB->myEntityID);
		}
	}
}
