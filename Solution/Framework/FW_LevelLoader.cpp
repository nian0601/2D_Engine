#include "FW_LevelLoader.h"

#include "FW_EntityManager.h"
#include "FW_Components.h"

namespace FW_LevelLoader
{
	void SetDataFolder(const char* aFolderName)
	{
		ourLevelPath = aFolderName;
		ourLevelPath += "/Data/Levels/";

		ourEntityPath = aFolderName;
		ourEntityPath += "/Data/Entities/";
	}

	void LoadLevel(FW_EntityManager& anEntityManager, const char* aLevelFileName)
	{
		FW_String filePath = ourLevelPath;
		filePath += aLevelFileName;
		filePath += ".level";

		FW_FileProcessor processor(filePath.GetBuffer(), FW_FileProcessor::READ);

		int entityCount = 0;
		processor.Process(entityCount);

		int serializeMethod = 0;
		for (int i = 0; i < entityCount; ++i)
		{
			processor.Process(serializeMethod);
			if (serializeMethod == 1)
			{
				FW_String entityFileName;
				Vector2f position;

				processor.Process(entityFileName);
				processor.Process(position);

				FW_String entityPath = ourEntityPath;
				entityPath += entityFileName;

				anEntityManager.CreateEntity(entityPath.GetBuffer(), position);
			}
		}
	}

	void SaveLevel(FW_EntityManager& anEntityManager, const char* aLevelFileName)
	{
		FW_GrowingArray<FW_EntityID> entities = anEntityManager.GetAllEntities();
		int entityCount = entities.Count();

		FW_String filePath = "Data/Levels/";
		filePath += aLevelFileName;
		filePath += ".level";

		FW_FileProcessor processor(filePath.GetBuffer(), FW_FileProcessor::WRITE);
		processor.Process(entityCount);

		// 0 == Nothing to load
		// 1 == Load translation + resource
		//
		// Replace with writing a Component-identifier instead (*NOT* the TypeID for the component, cant rely on the order that components are added and shit)
		//
		int serializeMethod = 0;
		for (FW_EntityID entityID : entities)
		{
			TranslationComponent* translation = anEntityManager.FindComponent<TranslationComponent>(entityID);
			FileResoureComponent* resource = anEntityManager.FindComponent<FileResoureComponent>(entityID);

			if (resource && translation)
			{
				serializeMethod = 1;
				processor.Process(serializeMethod);

				processor.Process(resource->myFileName);

				processor.Process(translation->myPosition.x);
				processor.Process(translation->myPosition.y);
			}
			else
			{
				serializeMethod = 0;
				processor.Process(serializeMethod);
			}
		}
	}

	FW_EntityID AddEntity(FW_EntityManager& anEntityManager, const Vector2f& aPosition, const char* aEntityFileName)
	{
		FW_String filePath = ourEntityPath;
		filePath += aEntityFileName;
		filePath += ".entity";

		FW_EntityID entityID = anEntityManager.CreateEntity(filePath.GetBuffer(), aPosition);
		return entityID;
	}
}