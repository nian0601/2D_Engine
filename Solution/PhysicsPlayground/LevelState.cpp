#include "LevelState.h"

#include <FW_EntityManager.h>
#include <FW_MessageQueue.h>
#include <FW_XMLParser.h>
#include <FW_Profiler.h>

#include "PhysicsWorld.h"
#include "FW_FileSystem.h"
#include "PhysicsSystem.h"
#include "CameraSystem.h"
#include <FW_RenderSystem.h>

LevelState::LevelState(FW_EntityManager& anEntityManager, PhysicsWorld& aPhysicsWorld)
	: myEntityManager(anEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
	myEntityManager.RegisterComponent<PhysicsComponent>();
	myEntityManager.RegisterComponent<PlayerComponent>();
	myEntityManager.RegisterComponent<GoalComponent>();
	myEntityManager.RegisterComponent<CameraControllerComponent>();

	FW_MessageQueue& messageQueue = myEntityManager.GetMessageQueue();
	messageQueue.RegisterMessageType<CollisionMessage>();

	messageQueue.SubscribeToMessage<FW_PreEntityRemovedMessage>(std::bind(&LevelState::OnPreEntityRemoved, this, std::placeholders::_1));
	messageQueue.SubscribeToMessage<CollisionMessage>(std::bind(&LevelState::OnCollision, this, std::placeholders::_1));

	LoadTiledLevel("level_one.tmx");
}

FW_StateStack::State::UpdateResult LevelState::OnUpdate()
{
	PhysicSystem::Run(myEntityManager, myPhysicsWorld);
	CameraSystem::Run(myEntityManager);
	FW_RenderSystem::Run(myEntityManager);

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

			LoadTiledLevel(myCurrentLevelInformation.myNextLevelName.GetBuffer());
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
					myEntityManager.QueueEntityRemoval(myPlayerEntity);
					myPlayerEntity = CreatePlayer();
				}
			}
		}
	}
}

void LevelState::LoadTiledLevel(const char* aLevelName)
{
	FW_PROFILE_FUNCTION();

	FW_String levelPath = "Levels/";
	levelPath += aLevelName;
	FW_XMLParser parser(levelPath);

	myCurrentLevelInformation.myLevelName = aLevelName;
	myCurrentLevelInformation.myTileSheet.myTiles.RemoveAll();

	if (parser.BeginElement("map"))
	{
		LoadMapProperties(parser);
		LoadMapTileSheets(parser);
		LoadMapLayers(parser);

		parser.EndElement();
	}

	myPlayerEntity = CreatePlayer();
}

void LevelState::LoadMapProperties(FW_XMLParser& aParser)
{
	FW_PROFILE_FUNCTION();

	if (aParser.BeginElement("properties"))
	{
		while (aParser.BeginElement("property"))
		{
			FW_String propertyName = aParser.GetStringAttribute("name");
			if (propertyName == "next_level")
				myCurrentLevelInformation.myNextLevelName = aParser.GetStringAttribute("value");

			aParser.EndElement();
		}
		aParser.EndElement();
	}
}

void LevelState::LoadMapTileSheets(FW_XMLParser& aParser)
{
	FW_PROFILE_FUNCTION();

	while (aParser.BeginElement("tileset"))
	{
		FW_String tilesetPath;
		FW_FileSystem::RemoveFileName(aParser.GetFilePath(), tilesetPath);

		FW_String tilesetSource = tilesetPath + aParser.GetStringAttribute("source");

		int firstGID = aParser.GetIntAttribute("firstgid");

		LoadTileSheet(tilesetSource.GetBuffer(), firstGID);

		aParser.EndElement();
	}
}

void LevelState::LoadMapLayers(FW_XMLParser& aParser)
{
	FW_PROFILE_FUNCTION();

	while (aParser.BeginElement("layer"))
	{
		if (aParser.BeginElement("data"))
		{
			while (aParser.BeginElement("chunk"))
			{
				int startX = aParser.GetIntAttribute("x");
				int startY = aParser.GetIntAttribute("y");

				int chunkWidth = aParser.GetIntAttribute("width");
				int chunkHeight = aParser.GetIntAttribute("height");

				FW_GrowingArray<int> tileIDs;
				for (int y = 0; y < chunkHeight; ++y)
				{
					aParser.GetRawDataAsInt(tileIDs, ",");

					for (int x = 0; x < tileIDs.Count(); ++x)
					{
						if (tileIDs[x] != 0)
						{
							for (Tilesheet::TileData& tile : myCurrentLevelInformation.myTileSheet.myTiles)
							{
								if (tile.myID == tileIDs[x])
								{
									Vector2f position;
									position.x = (startX + x) * 64.f;
									position.y = (startY + y) * 64.f;

									CreateTile(position, tile);
								}
							}
						}
					}
				}

				aParser.EndElement();
			}

			aParser.EndElement();
		}

		aParser.EndElement();
	}
}

void LevelState::LoadTileSheet(const char* aFilePath, int aFirstTileID)
{
	FW_XMLParser parser(aFilePath);

	FW_String tileBasePath;
	FW_FileSystem::RemoveFileName(aFilePath, tileBasePath);

	if (parser.BeginElement("tileset"))
	{
		FW_String tilesetName = parser.GetStringAttribute("name");
		int tileWidth = parser.GetIntAttribute("tilewidth");
		int tileHeight = parser.GetIntAttribute("tileheight");
		int tileCount = parser.GetIntAttribute("tilecount");

		if (parser.BeginElement("grid"))
		{
			FW_String orientation = parser.GetStringAttribute("orientation");
			int gridWidth = parser.GetIntAttribute("width");
			int gridHeight = parser.GetIntAttribute("height");

			parser.EndElement();
		}

		while (parser.BeginElement("tile"))
		{
			Tilesheet::TileData& tile = myCurrentLevelInformation.myTileSheet.myTiles.Add();
			tile.myID = parser.GetIntAttribute("id") + aFirstTileID;

			if (parser.BeginElement("properties"))
			{
				while (parser.BeginElement("property"))
				{
					FW_String propertyName = parser.GetStringAttribute("name");
					if (propertyName == "IsGoalPosition")
						tile.myIsGoal = true;

					if (propertyName == "IsSpawnPosition")
						tile.myIsSpawnPoint = true;

					parser.EndElement();
				}
				parser.EndElement();
			}

			if (parser.BeginElement("image"))
			{
				tile.myWidth = parser.GetIntAttribute("width");
				tile.myHeight = parser.GetIntAttribute("height");
				tile.myTexturePath = tileBasePath + parser.GetStringAttribute("source");

				if (tile.myIsSpawnPoint)
					myCurrentLevelInformation.myTileSheet.myPlayerTexturePath = tile.myTexturePath;

				parser.EndElement();
			}

			parser.EndElement();
		}

		parser.EndElement();
	}
}

FW_EntityID LevelState::CreateTile(const Vector2f& aPosition, const Tilesheet::TileData& someTileData)
{
	if (someTileData.myIsSpawnPoint)
	{
		myCurrentLevelInformation.mySpawnPosition = aPosition;
		return InvalidEntity;
	}

	FW_EntityID tile = myEntityManager.CreateEmptyEntity();
	RenderComponent& render = myEntityManager.AddComponent<RenderComponent>(tile);
	render.myTextureFileName = someTileData.myTexturePath;
	render.myTexture = FW_Renderer::GetTexture(render.myTextureFileName.GetBuffer());;
	render.mySpriteSize = render.myTexture.mySize;
	render.myTextureRect = MakeRect<int>(0, 0, render.myTexture.mySize.x, render.myTexture.mySize.y);

	TranslationComponent& translation = myEntityManager.AddComponent<TranslationComponent>(tile);
	translation.myPosition = aPosition;

	PhysicsComponent& physics = myEntityManager.AddComponent<PhysicsComponent>(tile);
	physics.myObject = new PhysicsObject(new AABBShape(Vector2f(64.f, 64.f)));

	physics.myObject->MakeStatic();
	physics.myObject->SetPosition(aPosition);
	physics.myObject->myColor = 0x33AAAAAA;
	physics.myObject->myEntityID = tile;

	if (someTileData.myIsGoal)
	{
		physics.myObject->MakeSensor();
		myEntityManager.AddComponent<GoalComponent>(tile);
	}

	myPhysicsWorld.AddObject(physics.myObject);

	return tile;
}

FW_EntityID LevelState::CreatePlayer()
{
	FW_EntityID player = myEntityManager.CreateEmptyEntity();
	RenderComponent& render = myEntityManager.AddComponent<RenderComponent>(player);
	render.myTextureFileName = myCurrentLevelInformation.myTileSheet.myPlayerTexturePath;
	render.myTexture = FW_Renderer::GetTexture(render.myTextureFileName.GetBuffer());;
	render.mySpriteSize = render.myTexture.mySize;
	render.myTextureRect = MakeRect<int>(0, 0, render.myTexture.mySize.x, render.myTexture.mySize.y);

	TranslationComponent& translation = myEntityManager.AddComponent<TranslationComponent>(player);
	translation.myPosition = myCurrentLevelInformation.mySpawnPosition;

	PhysicsComponent& physics = myEntityManager.AddComponent<PhysicsComponent>(player);
	physics.myObject = new PhysicsObject(new CircleShape(render.mySpriteSize.x * 0.5f));
	myPhysicsWorld.AddObject(physics.myObject);

	physics.myObject->SetPosition(translation.myPosition);
	physics.myObject->myRestitution = 0.f;
	physics.myObject->SetDensity(25.f);
	physics.myObject->SetInertia(0.f);
	physics.myObject->myColor = 0x33AAAAAA;
	physics.myObject->myEntityID = player;

	myEntityManager.AddComponent<PlayerComponent>(player);
	myEntityManager.AddComponent<CameraControllerComponent>(player);

	return player;
}
