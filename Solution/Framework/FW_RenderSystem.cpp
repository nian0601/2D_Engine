#include "FW_RenderSystem.h"
#include "FW_EntityManager.h"
#include "FW_Components.h"
#include "FW_Profiler.h"

void FW_RenderSystem::Run(FW_EntityManager& aEntityManger)
{
	FW_PROFILE_FUNCTION();

	Vector2i pos;

	FW_ComponentStorage<RenderComponent>& renderStorage = aEntityManger.GetComponentStorage<RenderComponent>();
	for (const RenderComponent& render : renderStorage)
	{
		if (const TranslationComponent* translation = aEntityManger.FindComponent<TranslationComponent>(render.myEntity))
		{
			pos.x = static_cast<int>(translation->myPosition.x);
			pos.y = static_cast<int>(translation->myPosition.y);
			FW_Renderer::RenderTexture(render.myTexture.myTextureID, pos, render.mySpriteSize, render.myTextureRect, translation->myOrientation);
		}
	}
}