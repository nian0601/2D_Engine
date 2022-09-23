#include "Spareparty.h"

#include <imgui/imgui-SFML.h>

#include <FW_Components.h>
#include <FW_RenderSystem.h>
#include <FW_Editor.h>
#include <FW_Time.h>
#include <FW_Math.h>

#include "PhysicsSystem.h"
#include "SparepartyComponents.h"
#include <FW_FileSystem.h>
#include <SFML_Renderer.h>
#include <FW_MessageQueue.h>
#include "LevelState.h"
#include <FW_Profiler.h>

Spareparty::Spareparty()
{
	myStateStack.PushMajorState(new LevelState(myEntityManager, myPhysicsWorld));
}

Spareparty::~Spareparty()
{
}

void Spareparty::Run()
{
	FW_PROFILE_FUNCTION();

	float delta = FW_Time::GetDelta();

	myStateStack.Update();

	if (myRenderPhysicsObjects)
	{
		myPhysicsWorld.RenderAllObjects();
		myPhysicsWorld.RenderContacts();
	}

	if (FW_Editor::BeginEditor(myEntityManager))
	{
		ImGui::Checkbox("Render Phys Objects", &myRenderPhysicsObjects);

		Vector2f mousePosition = FW_Input::GetMousePositionf();
		Vector2f mouseTilePosition = SnapPositionToGrid(mousePosition);

		FW_Renderer::RenderRect(MakeRect(mouseTilePosition, { 64.f, 64.f }), 0x44AAAAAA);

		//FW_EntityID entityUnderMouse = GetEntityUnderMouse();

		FW_Editor::EndEditor();
	}

	myEntityManager.EndFrame();
}

FW_EntityID Spareparty::GetEntityUnderMouse()
{
	const FW_ComponentStorage<RenderComponent>& renderStorage = myEntityManager.GetComponentStorage<RenderComponent>();
	for (const RenderComponent& renderComponent : renderStorage)
	{
		if (TranslationComponent* translateComponent = myEntityManager.FindComponent<TranslationComponent>(renderComponent.myEntity))
		{
			Vector2f size(float(renderComponent.mySpriteSize.x), float(renderComponent.mySpriteSize.y));
			Rectf rect = MakeRect(translateComponent->myPosition, size);

			if (Contains(rect, FW_Input::GetMousePositionf()))
				return renderComponent.myEntity;
		}
	}

	return InvalidEntity;
}

Vector2f Spareparty::SnapPositionToGrid(const Vector2f& aPosition) const
{
	const int width = 64;
	const int height = 50; // Seems wrong.. figure out why some tiles seem to be 50 high while some or 64?

	int tileX = static_cast<int>(aPosition.x + width/2) / width;
	int tileY = static_cast<int>(aPosition.y + height/2) / height;

	Vector2f position;
	position.x = static_cast<float>(tileX * width);
	position.y = static_cast<float>(tileY * height);
	return position;
}