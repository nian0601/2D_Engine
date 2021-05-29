#pragma once

#include "FW_Components.h"

//////////////////////////////////////////////////////////////////////////

struct MovementComponent
{
	DEFINE_COMPONENT(MovementComponent);

	Vector2f myDirection;
	float mySpeed;
};

template <>
inline void SerializeSpecificComponent(MovementComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myDirection);
	aFileProcessor.Process(aComponent.mySpeed);
}

template <>
inline void BuildSpecificComponentUI(MovementComponent& aComponent)
{
	ImGui::DragFloat2("Direction", &aComponent.myDirection.x, 0.01f, -1.f, 1.f);
	ImGui::DragFloat("Speed", &aComponent.mySpeed, 0.1f, 0.0f, 500.0f);
}

//////////////////////////////////////////////////////////////////////////

struct PlayerComponent
{
	DEFINE_COMPONENT(PlayerComponent);
};

//////////////////////////////////////////////////////////////////////////

inline void BuildCollisionFlagCheckbox(int& someCollisionFlags, CollisionComponent::Flags aFlag, const char* aLabel)
{
	bool temp = (someCollisionFlags & aFlag) > 0;
	if (ImGui::Checkbox(aLabel, &temp))
		someCollisionFlags ^= aFlag;
}

template <>
inline void BuildSpecificComponentUI(CollisionComponent& aComponent)
{
	if (ImGui::TreeNode("Collision Flags"))
	{
		BuildCollisionFlagCheckbox(aComponent.myCollisionFlags, CollisionComponent::Player, "Player");
		BuildCollisionFlagCheckbox(aComponent.myCollisionFlags, CollisionComponent::Brick, "Brick");
		BuildCollisionFlagCheckbox(aComponent.myCollisionFlags, CollisionComponent::Ball, "Ball");
		BuildCollisionFlagCheckbox(aComponent.myCollisionFlags, CollisionComponent::Powerup, "Powerup");
		BuildCollisionFlagCheckbox(aComponent.myCollisionFlags, CollisionComponent::Wall, "Wall");

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Collides With"))
	{
		BuildCollisionFlagCheckbox(aComponent.myCollidesWithFlags, CollisionComponent::Player, "Player");
		BuildCollisionFlagCheckbox(aComponent.myCollidesWithFlags, CollisionComponent::Brick, "Brick");
		BuildCollisionFlagCheckbox(aComponent.myCollidesWithFlags, CollisionComponent::Ball, "Ball");
		BuildCollisionFlagCheckbox(aComponent.myCollidesWithFlags, CollisionComponent::Powerup, "Powerup");
		BuildCollisionFlagCheckbox(aComponent.myCollidesWithFlags, CollisionComponent::Wall, "Wall");

		ImGui::TreePop();
	}
}

//////////////////////////////////////////////////////////////////////////

struct HealthComponent
{
	DEFINE_COMPONENT(HealthComponent);

	int myHealth;
};

template <>
inline void SerializeSpecificComponent(HealthComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myHealth);
}

template <>
inline void BuildSpecificComponentUI(HealthComponent& aComponent)
{
	ImGui::DragInt("Health", &aComponent.myHealth, 1, 0, 10);
}

//////////////////////////////////////////////////////////////////////////

struct BallComponent
{
	DEFINE_COMPONENT(BallComponent);
};

//////////////////////////////////////////////////////////////////////////

struct KillBallComponent
{
	DEFINE_COMPONENT(KillBallComponent);
};

//////////////////////////////////////////////////////////////////////////

struct PowerUpComponent
{
	DEFINE_COMPONENT(PowerUpComponent);

	enum Type
	{
		BALL_SPEED_UP,
		BALL_SPEED_DOWN,
		BALL_SPLIT
	};

	int myMagnitude = 0;
	int myType = 0;
};

template <>
inline void SerializeSpecificComponent(PowerUpComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myMagnitude);
	aFileProcessor.Process(aComponent.myType);
}

template <>
inline void BuildSpecificComponentUI(PowerUpComponent& aComponent)
{
	const char* items[] = { "Ball Speed", "Ball Slow", "Ball Split" };
	ImGui::Combo("Type", &aComponent.myType, items, IM_ARRAYSIZE(items));
	ImGui::DragInt("Magnitude", &aComponent.myMagnitude, 1, 0, 100);
}
