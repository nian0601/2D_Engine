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
	myEntityManager.RegisterComponent<GoalComponent>();

	FW_MessageQueue& messageQueue = myEntityManager.GetMessageQueue();
	messageQueue.RegisterMessageType<CollisionMessage>();

	messageQueue.SubscribeToMessage<FW_PreEntityRemovedMessage>(std::bind(&Spareparty::OnPreEntityRemoved, this, std::placeholders::_1));
	messageQueue.SubscribeToMessage<CollisionMessage>(std::bind(&Spareparty::OnCollision, this, std::placeholders::_1));


	myCurrentLevelID = 0;
	LoadLevel(myCurrentLevelID);
}

Spareparty::~Spareparty()
{
}

void Spareparty::Run()
{
	float delta = FW_Time::GetDelta();

	PhysicSystem::Run(myEntityManager, myPhysicsWorld);
	FW_RenderSystem::Run(myEntityManager);

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

void Spareparty::LoadLevel(int aLevelID)
{
	myEntityManager.QueueRemovalAllEntities();

	if (aLevelID == 0)
	{
		int map[10][10] =
		{
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0,89, 0, 0, 0, 0, 0, 0,99, 0},
			{ 0, 7, 0, 0, 0, 0, 4, 5, 5, 6},
			{ 5, 3, 5, 5, 5, 5, 3, 3, 3, 3}
		};
		LoadLevel(map);
	}
	else if (aLevelID == 1)
	{
		int map[10][10] =
		{
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 0,99, 0, 0, 0, 0, 0, 0, 0, 0},
			{ 4, 5, 6, 0, 0, 0, 0, 0, 0, 0},
			{ 3, 3, 3, 0, 0, 0, 0, 0, 0, 0},
			{ 3, 3, 3, 0, 0, 0, 0, 0, 0, 0},
			{ 3, 3, 3, 0, 0, 0, 0, 0, 0, 0},
			{ 3, 3, 3, 0, 0, 0, 0,89, 0, 0},
			{ 3, 3, 3, 5, 5, 5, 5, 5, 5, 5}
		};
		LoadLevel(map);
	}
}

void Spareparty::LoadLevel(int aMapData[10][10])
{
	Vector2f position = { 50.f, 50.f };
	for (int y = 0; y < 10; ++y)
	{
		for (int x = 0; x < 10; ++x)
		{
			int tileID = aMapData[y][x];
			if (tileID != 0)
			{
				if (tileID == 99)
					CreateGoal(position);
				else if (tileID == 89)
					CreatePlayer(position);
				else
					CreateTile(position, tileID);
			}

			position.x += 64.f;
		}

		position.x = 50.f;
		//position.y += 64.f;
		position.y += 64.f;
	}
}

FW_EntityID Spareparty::CreateTile(const Vector2f& aPosition, int aTileID)
{
	FW_String textureFilePath = "tiles/tile";
	if (aTileID < 10)
		textureFilePath += "0";
	textureFilePath += aTileID;
	textureFilePath += ".png";

	FW_Renderer::Texture texture = FW_Renderer::GetTexture(textureFilePath.GetBuffer());
	return CreateTile(aPosition, texture, textureFilePath.GetBuffer());
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

FW_EntityID Spareparty::CreateGoal(const Vector2f& aPosition)
{
	FW_String textureFilePath = "flagGreen_up.png";

	FW_Renderer::Texture texture = FW_Renderer::GetTexture(textureFilePath.GetBuffer());

	FW_EntityID goal = myEntityManager.CreateEmptyEntity();
	RenderComponent& render = myEntityManager.AddComponent<RenderComponent>(goal);
	render.myTextureFileName = textureFilePath.GetBuffer();
	render.myTexture = FW_Renderer::GetTexture(textureFilePath.GetBuffer());
	render.mySpriteSize = render.myTexture.mySize;
	render.myTextureRect = MakeRect<int>(0, 0, render.myTexture.mySize.x, render.myTexture.mySize.y);

	TranslationComponent& translation = myEntityManager.AddComponent<TranslationComponent>(goal);
	translation.myPosition = aPosition;

	PhysicsComponent& physics = myEntityManager.AddComponent<PhysicsComponent>(goal);
	physics.myObject = new PhysicsObject(new AABBShape(Vector2f(64.f, 64.f)));
	physics.myObject->SetPosition(aPosition);
	physics.myObject->myColor = 0x33AAAAAA;
	physics.myObject->MakeStatic();
	physics.myObject->MakeSensor();
	physics.myObject->myEntityID = goal;

	myPhysicsWorld.AddObject(physics.myObject);

	myEntityManager.AddComponent<GoalComponent>(goal);

	return goal;
}

FW_EntityID Spareparty::CreatePlayer(const Vector2f& aPosition)
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
	physics.myObject = new PhysicsObject(new CircleShape(render.mySpriteSize.x * 0.5f));
	physics.myObject->SetPosition(aPosition);
	physics.myObject->myColor = 0x33AAAAAA;
	physics.myObject->myRestitution = 0.f;
	physics.myObject->SetDensity(25.f);
	physics.myObject->SetInertia(0.f);
	physics.myObject->myEntityID = player;

	myEntityManager.AddComponent<PlayerComponent>(player);

	myPhysicsWorld.AddObject(physics.myObject);

	return player;
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
		myPhysicsWorld.RemoveObject(physics->myObject);
		delete physics->myObject;
		physics->myObject = nullptr;
	}
}

void Spareparty::OnCollision(const CollisionMessage& aMessage)
{
	if (PlayerComponent* player = myEntityManager.FindComponent<PlayerComponent>(aMessage.myFirstEntity))
	{
		if (GoalComponent* goal = myEntityManager.FindComponent<GoalComponent>(aMessage.mySecondEntity))
		{
			myEntityManager.QueueRemovalAllEntities();
			myEntityManager.FlushEntityRemovals();

			if (myCurrentLevelID == 0)
				myCurrentLevelID = 1;
			else
				myCurrentLevelID = 0;

			LoadLevel(myCurrentLevelID);
		}
		else
		{
			if (PhysicsComponent* physics = myEntityManager.FindComponent<PhysicsComponent>(aMessage.myFirstEntity))
			{
				bool isUpright = fabs(physics->myObject->myOrientation) < 0.2f;
				float speedLimit = isUpright ? 150.f : 10.f;

				float speed = Length(physics->myObject->myPreviousVelocity);
				if (speed > speedLimit)
				{
					myEntityManager.QueueRemovalAllEntities();
					myEntityManager.FlushEntityRemovals();

					LoadLevel(myCurrentLevelID);
				}
			}
		}
	}
}
