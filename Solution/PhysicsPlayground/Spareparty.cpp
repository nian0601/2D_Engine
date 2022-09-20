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
	myEntityManager.RegisterComponent<PhysicsComponent>();
	myEntityManager.RegisterComponent<PlayerComponent>();
	myEntityManager.RegisterComponent<GoalComponent>();

	FW_MessageQueue& messageQueue = myEntityManager.GetMessageQueue();
	messageQueue.RegisterMessageType<CollisionMessage>();

	myStateStack.PushMajorState(new LevelState(myEntityManager, myPhysicsWorld));
}

Spareparty::~Spareparty()
{
}

void Spareparty::Run()
{
	FW_PROFILE_FUNCTION();

	float delta = FW_Time::GetDelta();

	PhysicSystem::Run(myEntityManager, myPhysicsWorld);
	FW_RenderSystem::Run(myEntityManager);

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

		FW_EntityID entityUnderMouse = GetEntityUnderMouse();
		if (entityUnderMouse != InvalidEntity)
		{
			if (FW_Input::WasMouseReleased(FW_Input::MouseButton::LEFTMB))
			{
				FW_GrowingArray<FW_EntityID> entities;
				entities.Add(entityUnderMouse);
				FW_Editor::SetSelectedEntities(entities);
			}
		}
		else
		{
			if (FW_Input::IsMouseDown(FW_Input::MouseButton::LEFTMB))
			{
				if (mySelectedTexture.myTextureID != -1)
				{
					CreateTile(SnapPositionToGrid(FW_Input::GetMousePositionf()), mySelectedTexture);
				}
			}
			else if (FW_Input::WasMouseReleased(FW_Input::MouseButton::LEFTMB))
			{
				FW_Editor::ClearSelectedEntities();
			}
			else if (FW_Input::WasMouseReleased(FW_Input::MouseButton::RIGHTMB))
			{
				FW_GrowingArray<Vector2f> newEntityPositions;
				newEntityPositions.Add(mouseTilePosition);
				FW_Editor::NewEntityPopup(newEntityPositions);
			}
		}

		if (myTileTextures.IsEmpty())
		{
			FW_GrowingArray<FW_FileSystem::FileInfo> tileInfos;
			FW_FileSystem::GetAllFilesFromDirectory("tiles", tileInfos);
		
			for (const FW_FileSystem::FileInfo& info : tileInfos)
			{
				myTileTextures.Add(FW_Renderer::GetTexture(info.myFilePath.GetBuffer()));
			}
		}

		if (ImGui::CollapsingHeader("Tiles"))
		{
			for (int i = 0; i < myTileTextures.Count(); ++i)
			{
				if (i % 3 != 0)
					ImGui::SameLine();

				if (const sf::Texture* texture = SFML_Renderer::GetSFMLTexture(myTileTextures[i].myTextureID))
				{
					if (ImGui::ImageButton(*texture))
					{
						mySelectedTexture = myTileTextures[i];
					}

				}
			}
		}

		FW_Editor::EndEditor();
	}

	myEntityManager.EndFrame();
}

FW_EntityID Spareparty::CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName /*= ""*/)
{
	FW_EntityID tile = myEntityManager.CreateEmptyEntity();
	RenderComponent& render = myEntityManager.AddComponent<RenderComponent>(tile);
	render.myTextureFileName = aTextureFileName;
	render.myTexture = aTileTexture;
	render.mySpriteSize = render.myTexture.mySize;
	render.myTextureRect = MakeRect<int>(0, 0, render.myTexture.mySize.x, render.myTexture.mySize.y);

	TranslationComponent& translation = myEntityManager.AddComponent<TranslationComponent>(tile);
	translation.myPosition = aPosition;

	PhysicsComponent& physics = myEntityManager.AddComponent<PhysicsComponent>(tile);
	physics.myObject = new PhysicsObject(new AABBShape(Vector2f(64.f, 64.f)));
	physics.myObject->SetPosition(aPosition);
	physics.myObject->myColor = 0x33AAAAAA;
	physics.myObject->MakeStatic();
	physics.myObject->myEntityID = tile;

	myPhysicsWorld.AddObject(physics.myObject);

	return tile;
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