#pragma once

#include <vector>
#include <unordered_map>
#include "FW_FileProcessor.h"
#include "FW_Includes.h"


class FW_IComponentStorage
{
public:
	virtual void LoadComponent(FW_EntityID anEntity, FW_FileProcessor& aFileProcessor) = 0;
	virtual void SaveComponent(FW_EntityID anEntity, FW_FileProcessor& aFileProcessor) = 0;

	virtual void RemoveComponent(FW_EntityID anEntity) = 0;
	virtual void RemoveAllComponents() = 0;

	virtual void BuildModifyComponentUI(FW_EntityID anEntity) = 0;
	virtual void BuildAddComponentUI(FW_EntityID anEntity) = 0;
	virtual const FW_String& GetComponentIdentifierString() const = 0;
};

template <typename Component>
class FW_ComponentStorage : public FW_IComponentStorage
{
public:
	void LoadComponent(FW_EntityID anEntity, FW_FileProcessor& aFileProcessor) override;
	void SaveComponent(FW_EntityID anEntity, FW_FileProcessor& aFileProcessor) override;

	Component& AddComponent(FW_EntityID anEntity);
	void RemoveComponent(FW_EntityID anEntity)
	{
		for (int i = 0; i < myNextFreeComponent; ++i)
		{
			if (anEntity == myComponents[i].myEntity)
			{
				myComponents[i] = myComponents[myNextFreeComponent - 1];
				--myNextFreeComponent;
				break;
			}
		}
	}

	void RemoveAllComponents()
	{
		myNextFreeComponent = 0;
	}

	void BuildModifyComponentUI(FW_EntityID anEntity) override
	{
		for (int i = 0; i < myNextFreeComponent; ++i)
		{
			if (anEntity == myComponents[i].myEntity)
			{
				if (ImGui::CollapsingHeader(GetComponentIdentifierString().GetBuffer()))
				{
					BuildSpecificComponentUI(myComponents[i]);
					ImGui::Separator();

					if (ImGui::Button("Remove"))
					{
						myComponents[i] = myComponents[myNextFreeComponent - 1];
						--myNextFreeComponent;
					}
				}

				break;
			}
		}
	}

	void BuildAddComponentUI(FW_EntityID anEntity) override
	{
		if (FindComponent(anEntity))
			return;

		if (ImGui::Button(GetComponentIdentifierString().GetBuffer()))
		{
			AddComponent(anEntity);
		}
	}

	const FW_String& GetComponentIdentifierString() const { return Component::GetIdentifierString(); }

	Component* FindComponent(FW_EntityID anEntity);

	int Count() const { return myNextFreeComponent; }

	Component& operator[](int aIndex);
	const Component& operator[](int aIndex) const;

	Component* begin() { return &myComponents[0]; }
	const Component* begin() const { return &myComponents[0]; }
	Component* end() { return &myComponents[myNextFreeComponent]; }
	const Component* end() const { return &myComponents[myNextFreeComponent]; }

private:
	Component myComponents[MAX_COMPONENTS];
	int myNextFreeComponent = 0;
	// Might want to look into re-adding a EntityID-to-Component hashmap again, but wont be able to use std::unordered_map, its waaaaaay to slow in debug somehow...
};

template <typename Component>
void FW_ComponentStorage<Component>::LoadComponent(FW_EntityID anEntity, FW_FileProcessor& aFileProcessor)
{
	FW_ASSERT(myNextFreeComponent < MAX_COMPONENTS, "Too many components");

	Component& component = myComponents[myNextFreeComponent++];
	component.myEntity = anEntity;
	SerializeSpecificComponent(component, aFileProcessor);
}

template <typename Component>
void FW_ComponentStorage<Component>::SaveComponent(FW_EntityID anEntity, FW_FileProcessor& aFileProcessor)
{
	if (Component* component = FindComponent(anEntity))
	{
		aFileProcessor.Process(GetComponentIdentifierString());
		SerializeSpecificComponent(*component, aFileProcessor);
	}
}

template <typename Component>
Component& FW_ComponentStorage<Component>::AddComponent(FW_EntityID anEntity)
{
	FW_ASSERT(myNextFreeComponent < MAX_COMPONENTS, "Too many components");

	Component& component = myComponents[myNextFreeComponent++];
	component.myEntity = anEntity;

	return component;
}

template <typename Component>
Component* FW_ComponentStorage<Component>::FindComponent(FW_EntityID anEntity)
{
	for (int i = 0; i < myNextFreeComponent; ++i)
	{
		if (myComponents[i].myEntity == anEntity)
			return &myComponents[i];
	}

	return nullptr;
}

template <typename Component>
Component& FW_ComponentStorage<Component>::operator[](int aIndex)
{
	assert(aIndex >= 0 && "Invalid Index");
	assert(aIndex < myNextFreeComponent && "Invalid Index");
	return myComponents[aIndex];
}

template <typename Component>
const Component& FW_ComponentStorage<Component>::operator[](int aIndex) const
{
	assert(aIndex >= 0 && "Invalid Index");
	assert(aIndex < myNextFreeComponent && "Invalid Index");
	return myComponents[aIndex];
}