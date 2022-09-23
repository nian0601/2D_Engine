#include "CameraSystem.h"

#include "SparepartyComponents.h"

#include <FW_EntityManager.h>

namespace CameraSystem
{
	void Run(FW_EntityManager& aEntityManager)
	{
		FW_ComponentStorage<CameraControllerComponent>& cameraComponents = aEntityManager.GetComponentStorage<CameraControllerComponent>();
		for (CameraControllerComponent& cameraController : cameraComponents)
		{
			if (TranslationComponent* translation = aEntityManager.FindComponent<TranslationComponent>(cameraController.myEntity))
				FW_Renderer::GetCamera().myPosition = translation->myPosition;
		}
	}
}