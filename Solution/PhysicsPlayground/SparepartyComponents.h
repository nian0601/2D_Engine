#pragma once

#include <FW_Components.h>

struct PhysicsObject;
struct PhysicsComponent
{
	DEFINE_COMPONENT(PhysicsComponent);

	PhysicsObject* myObject;
};

//template <>
//inline void SerializeSpecificComponent(TranslationComponent& aComponent, FW_FileProcessor& aFileProcessor)
//{
//	aFileProcessor.Process(aComponent.myPosition);
//
//	if (aFileProcessor.IsReading())
//		aComponent.mySpawnPosition = aComponent.myPosition;
//}
//
//template <>
//inline void BuildSpecificComponentUI(TranslationComponent& aComponent)
//{
//	ImGui::Text("Translation");
//	ImGui::DragFloat2("Position", &aComponent.myPosition.x, 0.1f, 0.0f, 1000.0f);
//	ImGui::Spacing();
//}

struct PlayerComponent
{
	DEFINE_COMPONENT(PlayerComponent);
};

struct GoalComponent
{
	DEFINE_COMPONENT(GoalComponent);
};

struct CameraControllerComponent
{
	DEFINE_COMPONENT(GoalComponent);
};



//////////////////////////////////////////////////////////////////////////

struct CollisionMessage
{
	FW_EntityID myFirstEntity;
	FW_EntityID mySecondEntity;
};