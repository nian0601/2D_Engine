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

Spareparty::Spareparty()
{
	myEntityManager.RegisterComponent<PhysicsComponent>();
	myEntityManager.RegisterComponent<PlayerComponent>();

	FW_MessageQueue& messageQueue = myEntityManager.GetMessageQueue();
	messageQueue.SubscribeToMessage<FW_PreEntityRemovedMessage>(std::bind(&Spareparty::OnPreEntityRemoved, this, std::placeholders::_1));

	int map[10][10] =
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
		{ 0, 0, 0, 0,15,16, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 7, 0, 9, 5,10, 4, 5, 5, 6},
		{ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}
	};

	Vector2f position = { 50.f, 50.f };
	for (int y = 0; y < 10; ++y)
	{
		for (int x = 0; x < 10; ++x)
		{
			if (map[y][x] != 0)
			{
				CreateTile(position, map[y][x]);
			}

			position.x += 64.f;
		}

		position.x = 50.f;
		//position.y += 64.f;
		position.y += 64.f;
	}

	CreatePlayer({ 400.f, 280.f });
}

Spareparty::~Spareparty()
{

}

void Spareparty::Run()
{
	float delta = FW_Time::GetDelta();

	PhysicSystem::Run(myEntityManager, myPhysicsWorld);
	FW_RenderSystem::Run(myEntityManager);

	if(myRenderPhysicsObjects)
		myPhysicsWorld.RenderAllObjects();

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

void Spareparty::CreateTile(const Vector2f& aPosition, int aTileID)
{
	FW_String textureFilePath = "tiles/tile";
	if (aTileID < 10)
		textureFilePath += "0";
	textureFilePath += aTileID;
	textureFilePath += ".png";

	FW_Renderer::Texture texture = FW_Renderer::GetTexture(textureFilePath.GetBuffer());
	CreateTile(aPosition, texture, textureFilePath.GetBuffer());
}

void Spareparty::CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName /*= ""*/)
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
	physics.myObject->SetStatic();

	myPhysicsWorld.AddObject(physics.myObject);
}

void Spareparty::CreatePlayer(const Vector2f& aPosition)
{
	FW_EntityID player = myEntityManager.CreateEmptyEntity();
	RenderComponent& render = myEntityManager.AddComponent<RenderComponent>(player);
	render.myTextureFileName = "playerBlue_stand.png";
	render.myTexture = FW_Renderer::GetTexture(render.myTextureFileName.GetBuffer());
	render.mySpriteSize = render.myTexture.mySize;
	render.myTextureRect = MakeRect<int>(0, 0, render.myTexture.mySize.x, render.myTexture.mySize.y);

	TranslationComponent& translation = myEntityManager.AddComponent<TranslationComponent>(player);
	translation.myPosition = aPosition;

	PhysicsComponent& physics = myEntityManager.AddComponent<PhysicsComponent>(player);
	physics.myObject = new PhysicsObject(new AABBShape(render.mySpriteSize));
	physics.myObject->SetPosition(aPosition);
	physics.myObject->myColor = 0x33AAAAAA;
	physics.myObject->SetInertia(0.f);

	myEntityManager.AddComponent<PlayerComponent>(player);

	myPhysicsWorld.AddObject(physics.myObject);
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
	const int height = 64; // Seems wrong.. figure out why some tiles seem to be 50 high while some or 64?

	int tileX = static_cast<int>(aPosition.x + width/2) / width;
	int tileY = static_cast<int>(aPosition.y + height/2) / height;

	Vector2f position;
	position.x = static_cast<float>(tileX * width);
	position.y = static_cast<float>(tileY * height);
	return position;
}

void Spareparty::OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage)
{
	if (PhysicsComponent* physics = myEntityManager.FindComponent<PhysicsComponent>(aMessage.myEntity))
	{
		// Should the physworld own the objects?
		// Probably not else it will be hard (impossible?) to support temporarily removing an object?
		myPhysicsWorld.RemoveObject(physics->myObject);
		delete physics->myObject; 
	}
}
