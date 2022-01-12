#include "stdafx.h"

#include "PowerupSystem.h"
#include "FW_EntityManager.h"
#include "FW_Math.h"
#include "FW_LevelLoader.h"

namespace PowerupSystem
{
	void OnBallSpeedModify(FW_EntityManager& aEntityManager, int aAmount)
	{
		FW_ComponentStorage<BallComponent>& ballStorage = aEntityManager.GetComponentStorage<BallComponent>();
		for (BallComponent& ballComponent : ballStorage)
		{
			MovementComponent* movement = aEntityManager.FindComponent<MovementComponent>(ballComponent.myEntity);
			movement->mySpeed += aAmount;
		}
	}

	void OnBallSplit(FW_EntityManager& aEntityManager)
	{
		FW_GrowingArray<FW_EntityID> createdBalls;

		FW_ComponentStorage<BallComponent>& ballStorage = aEntityManager.GetComponentStorage<BallComponent>();
		for (BallComponent& ballComponent : ballStorage)
		{
			if (createdBalls.Find(ballComponent.myEntity) != -1)
				continue;

			TranslationComponent* oldTranslation = aEntityManager.FindComponent<TranslationComponent>(ballComponent.myEntity);

			FW_EntityID newBall = FW_LevelLoader::AddEntity(aEntityManager, oldTranslation->myPosition, "ball");
			createdBalls.Add(newBall);

			MovementComponent* newMovement = aEntityManager.FindComponent<MovementComponent>(newBall);
			MovementComponent* oldMovement = aEntityManager.FindComponent<MovementComponent>(ballComponent.myEntity);

			newMovement->mySpeed = oldMovement->mySpeed;
			newMovement->myDirection = ReflectAround(oldMovement->myDirection, { 0.f, 1.f });
		}
	}

	const char* GetRandomPowerup()
	{
		int powerupIndex = rand() % 3;

		switch (powerupIndex)
		{
		case 0: return "powerup_ball_speedup";
		case 1: return "powerup_ball_speeddown";
		default: return "powerup_ball_splitt";
			break;
		}
	}

	void OnCollision(FW_EntityManager& aEntityManager, const FW_CollisionMessage& aMessage)
	{
		PowerUpComponent* powerupComponent = aEntityManager.FindComponent<PowerUpComponent>(aMessage.myEntity);
		if (!powerupComponent)
			return;

		if (aEntityManager.FindComponent<KillBallComponent>(aMessage.myEntityThatHitMe))
		{
			aEntityManager.QueueEntityRemoval(aMessage.myEntity);
			return;
		}

		switch (powerupComponent->myType)
		{
		case PowerUpComponent::BALL_SPEED_DOWN: OnBallSpeedModify(aEntityManager, -powerupComponent->myMagnitude); break;
		case PowerUpComponent::BALL_SPEED_UP: OnBallSpeedModify(aEntityManager, powerupComponent->myMagnitude); break;
		case PowerUpComponent::BALL_SPLIT: OnBallSplit(aEntityManager); break;
		default:
			FW_ASSERT_ALWAYS("Unhandled Powerup");
			break;
		}

		aEntityManager.QueueEntityRemoval(aMessage.myEntity);
	}

	void OnEntityRemoved(FW_EntityManager& aEntityManager, FW_EntityID anEntityID)
	{
		if (HealthComponent* healthComponent = aEntityManager.FindComponent<HealthComponent>(anEntityID))
		{
			float random = FW_RandFloat();
			if (random > 0.50)
			{
				TranslationComponent* translationToCopy = aEntityManager.FindComponent<TranslationComponent>(anEntityID);
				FW_LevelLoader::AddEntity(aEntityManager, translationToCopy->myPosition, GetRandomPowerup());
			}
		}
	}
}