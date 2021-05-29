#pragma once
#include "FW_String.h"
#include "imgui\imgui.h"
#include "FW_FileProcessor.h"
#include "FW_Includes.h"

template <typename Component>
void BuildSpecificComponentUI(Component& /*aComponent*/) { }

template <typename Component>
void SerializeSpecificComponent(Component& /*aComponent*/, FW_FileProcessor& /*aFileProcessor*/) {}

#define DEFINE_COMPONENT(aComponent) \
static const FW_String& GetIdentifierString() \
{\
	static FW_String identifier(#aComponent);\
	return identifier;\
} \
FW_EntityID myEntity;

//////////////////////////////////////////////////////////////////////////

struct TranslationComponent
{
	DEFINE_COMPONENT(TranslationComponent);

	Vector2f myPosition;

	// Worth its own component?
	Vector2f mySpawnPosition;
};

template <>
inline void SerializeSpecificComponent(TranslationComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myPosition);

	if (aFileProcessor.IsReading())
		aComponent.mySpawnPosition = aComponent.myPosition;
}

template <>
inline void BuildSpecificComponentUI(TranslationComponent& aComponent)
{
	ImGui::Text("Translation");
	ImGui::DragFloat2("Position", &aComponent.myPosition.x, 0.1f, 0.0f, 1000.0f);
	ImGui::Spacing();
}

//////////////////////////////////////////////////////////////////////////

struct RenderComponent
{
	DEFINE_COMPONENT(RenderComponent);

	Recti myTextureRect;
	Vector2i mySpriteSize;
	FW_Renderer::Texture myTexture;
};

template <>
inline void SerializeSpecificComponent(RenderComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myTextureRect);
	aFileProcessor.Process(aComponent.mySpriteSize);
}

template <>
inline void BuildSpecificComponentUI(RenderComponent& aComponent)
{
	ImGui::DragInt2("Size", &aComponent.mySpriteSize.x, 1, 0, 500);
	if (ImGui::DragInt2("TopLeft", &aComponent.myTextureRect.myTopLeft.x, 1, 0, 1000) || ImGui::DragInt2("BottomRight", &aComponent.myTextureRect.myBottomRight.x, 1, 0, 1000))
	{
		aComponent.myTextureRect = MakeRect(
			aComponent.myTextureRect.myTopLeft.x,
			aComponent.myTextureRect.myTopLeft.y,
			aComponent.myTextureRect.myBottomRight.x,
			aComponent.myTextureRect.myBottomRight.y);
	}
}

//////////////////////////////////////////////////////////////////////////

struct CollisionComponent
{
	DEFINE_COMPONENT(CollisionComponent);

	Rectf myRect;
	Rectf myPreviousRect;

	enum Flags
	{
		Player = 1 << 0,
		Brick = 1 << 1,
		Ball = 1 << 2,
		Powerup = 1 << 3,
		Wall = 1 << 4,
	};

	int myCollisionFlags;
	int myCollidesWithFlags;
};

template <>
inline void SerializeSpecificComponent(CollisionComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myRect);
	aFileProcessor.Process(aComponent.myCollisionFlags);
	aFileProcessor.Process(aComponent.myCollidesWithFlags);

	if (aFileProcessor.IsReading())
		aComponent.myPreviousRect = aComponent.myRect;
}

//////////////////////////////////////////////////////////////////////////

struct FileResoureComponent
{
	DEFINE_COMPONENT(FileResoureComponent);

	FW_String myFileName;
};

template <>
inline void SerializeSpecificComponent(FileResoureComponent& aComponent, FW_FileProcessor& aFileProcessor)
{
	aFileProcessor.Process(aComponent.myFileName);
}

template <>
inline void BuildSpecificComponentUI(FileResoureComponent& aComponent)
{
	ImGui::Text(aComponent.myFileName.GetBuffer());
}

//////////////////////////////////////////////////////////////////////////