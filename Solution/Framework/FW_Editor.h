#pragma once

#include "FW_Includes.h"

class FW_EntityManager;
namespace FW_Editor
{
	bool BeginEditor(FW_EntityManager& aEntityStorage);
	void NewEntityPopup(const FW_GrowingArray<Vector2f>& somePositions);
	void EditEntity(FW_EntityID anEntity);
	void EndEditor();

	void SetSelectedEntities(const FW_GrowingArray<FW_EntityID>& someEntities);
	void ClearSelectedEntities();
}