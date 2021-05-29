#pragma once
#include "FW_Includes.h"
#include "FW_Vector2.h"

class FW_EntityManager;
namespace FW_LevelLoader
{
	static const char* ourLevelPath = "Data/Levels/";

	void LoadLevel(FW_EntityManager& anEntityManager, const char* aLevelFileName);
	void SaveLevel(FW_EntityManager& anEntityManager, const char* aLevelFileName);

	FW_EntityID AddEntity(FW_EntityManager& anEntityManager, const Vector2f& aPosition, const char* aEntityFileName);
}
