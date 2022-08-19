#include "LevelState.h"

#include <FW_EntityManager.h>
#include <FW_MessageQueue.h>

#include "PhysicsWorld.h"

LevelState::LevelState(FW_EntityManager& anEntityManager, PhysicsWorld& aPhysicsWorld)
	: myEntityManager(anEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
	FW_MessageQueue& messageQueue = myEntityManager.GetMessageQueue();
	messageQueue.SubscribeToMessage<FW_PreEntityRemovedMessage>(std::bind(&LevelState::OnPreEntityRemoved, this, std::placeholders::_1));
	messageQueue.SubscribeToMessage<CollisionMessage>(std::bind(&LevelState::OnCollision, this, std::placeholders::_1));

	myCurrentLevelID = 0;
	LoadLevel(myCurrentLevelID);
}

FW_StateStack::State::UpdateResult LevelState::OnUpdate()
{
	return FW_StateStack::State::KEEP_STATE;
}

void LevelState::OnPreEntityRemoved(const FW_PreEntityRemovedMessage& aMessage)
{
	if (PhysicsComponent* physics = myEntityManager.FindComponent<PhysicsComponent>(aMessage.myEntity))
	{
		myPhysicsWorld.RemoveObject(physics->myObject);
		delete physics->myObject;
		physics->myObject = nullptr;
	}
}

void LevelState::OnCollision(const CollisionMessage& aMessage)
{
	if (PlayerComponent* player = myEntityManager.FindComponent<PlayerComponent>(aMessage.myFirstEntity))
	{
		if (GoalComponent* goal = myEntityManager.FindComponent<GoalComponent>(aMessage.mySecondEntity))
		{
			myEntityManager.QueueRemovalAllEntities();
			myEntityManager.FlushEntityRemovals();

			++myCurrentLevelID;
			if (myCurrentLevelID > 2)
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

void LevelState::LoadLevel(int aLevelID)
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
	else if (aLevelID == 2)
	{
		int map[10][10] =
		{
			{ 0, 0, 0, 0, 3, 3, 3, 3, 3, 3},
			{ 0, 0, 0, 0, 0, 3, 0, 0, 0, 3},
			{ 0, 0, 0, 0, 0, 3, 0, 0,99, 3},
			{ 0, 0, 3, 0, 0, 3, 0, 0, 3, 3},
			{ 0, 0, 3, 0, 0, 3, 0, 0, 3, 3},
			{ 0, 0, 3, 0, 0, 3, 0, 0, 3, 3},
			{ 0, 0, 3, 0, 0, 0, 0, 0, 3, 3},
			{ 0,89, 3, 0, 0, 0, 0, 0, 3, 3},
			{ 0, 7, 3, 0, 0, 0, 4, 5, 3, 3},
			{ 5, 3, 3, 5, 5, 5, 3, 3, 3, 3}
		};
		LoadLevel(map);
	}
}

void LevelState::LoadLevel(int aMapData[10][10])
{
	Vector2f position = { 0.f, 0.f };
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

		position.x = 64.f;
		//position.y += 64.f;
		position.y += 64.f;
	}
}

FW_EntityID LevelState::CreateTile(const Vector2f& aPosition, int aTileID)
{
	FW_String textureFilePath = "tiles/tile";
	if (aTileID < 10)
		textureFilePath += "0";
	textureFilePath += aTileID;
	textureFilePath += ".png";

	FW_Renderer::Texture texture = FW_Renderer::GetTexture(textureFilePath.GetBuffer());
	return CreateTile(aPosition, texture, textureFilePath.GetBuffer());
}

FW_EntityID LevelState::CreateTile(const Vector2f& aPosition, FW_Renderer::Texture aTileTexture, const char* aTextureFileName /*= ""*/)
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

FW_EntityID LevelState::CreateGoal(const Vector2f& aPosition)
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

FW_EntityID LevelState::CreatePlayer(const Vector2f& aPosition)
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