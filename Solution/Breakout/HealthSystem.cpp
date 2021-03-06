#include "stdafx.h"

#include "HealthSystem.h"
#include "FW_EntityManager.h"
#include "FW_AudioSystem.h"

namespace HealthSystem
{
	void Run(FW_EntityManager& aEntityManager)
	{
		FW_ComponentStorage<HealthComponent>& healthStorage = aEntityManager.GetComponentStorage<HealthComponent>();
		for (HealthComponent& health : healthStorage)
		{
			if (health.myHealth <= 0)
				aEntityManager.QueueEntityRemoval(health.myEntity);
		}
	}

	void OnCollision(FW_EntityManager& aEntityManager, const FW_CollisionMessage& aMessage, int aDamageSoundEffectID)
	{
		if (HealthComponent* health = aEntityManager.FindComponent<HealthComponent>(aMessage.myEntity))
		{
			health->myHealth -= 1;
			FW_AudioSystem::PlayClip(aDamageSoundEffectID);
		}
	}
}