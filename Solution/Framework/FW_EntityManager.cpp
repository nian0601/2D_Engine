#include "FW_EntityManager.h"

#include "FW_CollisionSystem.h"
#include "FW_Components.h"
#include "FW_MessageQueue.h"
#include "FW_Messages.h"
#include "FW_RenderSystem.h"

FW_EntityManager::FW_EntityManager()
{
	myMessageQueue = new FW_MessageQueue();
	myMessageQueue->RegisterMessageType<FW_CollisionMessage>();
	myMessageQueue->RegisterMessageType<FW_EntityCreatedMessage>();
	myMessageQueue->RegisterMessageType<FW_PreEntityRemovedMessage>();

	RegisterComponent<TranslationComponent>();
	RegisterComponent<RenderComponent>();
	RegisterComponent<FileResoureComponent>();
}

FW_EntityManager::~FW_EntityManager()
{
	delete myMessageQueue;
}

void FW_EntityManager::EndFrame()
{
	myMessageQueue->SendQueuedMessages();

	FlushEntityRemovals();
}

FW_EntityID FW_EntityManager::CreateEmptyEntity()
{
	FW_EntityID newEntity = myNextEntity++;
	myEntities.Add(newEntity);
	return newEntity;
}

FW_EntityID FW_EntityManager::CreateEntity(const char* aEntityFilePath, const Vector2f& aPosition)
{
	FW_FileProcessor fileProcessor(aEntityFilePath, FW_FileProcessor::READ);

	FW_EntityID newEntity = CreateEmptyEntity();

	FW_String componentIdentifierFromDisk;

	FW_GrowingArray<FW_IComponentStorage*>& componentStorages = GetAllComponentStorages();
	while (!fileProcessor.AtEndOfFile())
	{
		fileProcessor.Process(componentIdentifierFromDisk);

		for (FW_IComponentStorage* storage : componentStorages)
		{
			if (componentIdentifierFromDisk == storage->GetComponentIdentifierString())
			{
				storage->LoadComponent(newEntity, fileProcessor);
				continue;
			}
		}
	}

	myMessageQueue->InstantlySendMessage<FW_EntityCreatedMessage>({ newEntity, aPosition });

	return newEntity;
}

void FW_EntityManager::SaveEntity(FW_EntityID anEntity, const char* aEntityFilePath)
{
	FW_FileProcessor fileProcessor(aEntityFilePath, FW_FileProcessor::WRITE);

	FW_GrowingArray<FW_IComponentStorage*>& componentStorages = GetAllComponentStorages();
	for (FW_IComponentStorage* storage : componentStorages)
		storage->SaveComponent(anEntity, fileProcessor);
}

void FW_EntityManager::QueueEntityRemoval(FW_EntityID anEntity)
{
	for (FW_EntityID entityID : myEntitiesToRemove)
	{
		if (entityID == anEntity)
			return;
	}

	myEntitiesToRemove.Add(anEntity);
}

void FW_EntityManager::QueueRemovalAllEntities()
{
	for (FW_EntityID entityID : myEntities)
		QueueEntityRemoval(entityID);
}

void FW_EntityManager::FlushEntityRemovals()
{
	for (FW_EntityID entityID : myEntitiesToRemove)
	{
		myMessageQueue->InstantlySendMessage<FW_PreEntityRemovedMessage>({ entityID });

		for (FW_IComponentStorage* storage : myComponentStorages)
			storage->RemoveComponent(entityID);

		myEntities.RemoveCyclic(entityID);
	}

	myEntitiesToRemove.RemoveAll();
}

void FW_EntityManager::BuildComponentUI(FW_EntityID anEntity)
{
	for (FW_IComponentStorage* storage : myComponentStorages)
		storage->BuildModifyComponentUI(anEntity);

	for (FW_IComponentStorage* storage : myComponentStorages)
		storage->BuildAddComponentUI(anEntity);
}

void FW_EntityManager::RemoveEntity(FW_EntityID anEntityID)
{
	for (FW_IComponentStorage* storage : myComponentStorages)
		storage->RemoveComponent(anEntityID);

	myEntities.RemoveCyclic(anEntityID);
}

void FW_EntityManager::RemoveAllEntities()
{
	for (FW_IComponentStorage* storage : myComponentStorages)
		storage->RemoveAllComponents();

	myEntities.RemoveAll();
}