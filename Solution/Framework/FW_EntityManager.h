#pragma once

#include "FW_Includes.h"
#include "FW_ComponentStorage.h"
#include "FW_TypeID.h"

class FW_MessageQueue;

class FW_EntityManager
{
public:
	FW_EntityManager();
	~FW_EntityManager();
	void EndFrame();

	FW_EntityID CreateEntity(const char* aEntityFilePath, const Vector2f& aPosition);
	void SaveEntity(FW_EntityID anEntity, const char* aEntityFilePath);

	void QueueEntityRemoval(FW_EntityID anEntity);
	void QueueRemovalAllEntities();

	void FlushEntityRemovals();

	template <typename ComponentType>
	ComponentType& AddComponent(FW_EntityID anEntity);

	template <typename ComponentType>
	void RemoveComponent(FW_EntityID anEntity);

	template <typename ComponentType>
	ComponentType* FindComponent(FW_EntityID anEntity);

	template <typename ComponentType>
	FW_ComponentStorage<ComponentType>& GetComponentStorage();

	FW_GrowingArray<FW_IComponentStorage*>& GetAllComponentStorages() { return myComponentStorages; }

	template <typename ComponentType>
	void RegisterComponent();

	const FW_GrowingArray<FW_EntityID>& GetAllEntities() const { return myEntities; }
	void BuildComponentUI(FW_EntityID anEntity);

	FW_MessageQueue& GetMessageQueue() const { return *myMessageQueue; }

protected:
	FW_EntityID CreateEntity();
	
	void RemoveEntity(FW_EntityID anEntityID);
	void RemoveAllEntities();

	FW_MessageQueue* myMessageQueue;

private:
	struct BaseComponent {};

	FW_GrowingArray<FW_EntityID> myEntities;
	FW_GrowingArray<FW_EntityID> myEntitiesToRemove;
	FW_EntityID myNextEntity = 0;

	FW_GrowingArray<FW_IComponentStorage*> myComponentStorages;
};


template <typename ComponentType>
ComponentType& FW_EntityManager::AddComponent(FW_EntityID anEntity)
{
	FW_ComponentStorage<ComponentType>& storage = GetComponentStorage<ComponentType>();
	return storage.AddComponent(anEntity);
}

template <typename ComponentType>
void FW_EntityManager::RemoveComponent(FW_EntityID anEntity)
{
	FW_ComponentStorage<ComponentType>& storage = GetComponentStorage<ComponentType>();
	storage.RemoveComponent(anEntity);
}

template <typename ComponentType>
ComponentType* FW_EntityManager::FindComponent(FW_EntityID anEntity)
{
	FW_ComponentStorage<ComponentType>& storage = GetComponentStorage<ComponentType>();
	return storage.FindComponent(anEntity);
}

template <typename ComponentType>
FW_ComponentStorage<ComponentType>& FW_EntityManager::GetComponentStorage()
{
	int componentTypeID = FW_TypeID<BaseComponent>::GetID<ComponentType>();
	FW_ASSERT(componentTypeID < myComponentStorages.Count(), "Unregistered component");

	FW_IComponentStorage* storage = myComponentStorages[componentTypeID];
	FW_ASSERT(storage != nullptr, "Unregistered component!");

	return *static_cast<FW_ComponentStorage<ComponentType>*>(storage);
}

template <typename ComponentType>
void FW_EntityManager::RegisterComponent()
{
	int componentTypeID = FW_TypeID<BaseComponent>::GetID<ComponentType>();
	if (componentTypeID >= myComponentStorages.GetCapacity())
		myComponentStorages.Respace(myComponentStorages.GetCapacity() * 2);
	else
		FW_ASSERT(componentTypeID >= myComponentStorages.Count(), "Component already registered");

	myComponentStorages.Add(new FW_ComponentStorage<ComponentType>());
}