#include "LevelState.h"

#include <FW_EntityManager.h>
#include <FW_MessageQueue.h>
#include <FW_XMLParser.h>
#include <FW_Profiler.h>

#include "PhysicsWorld.h"
#include "FW_FileSystem.h"

LevelState::LevelState(FW_EntityManager& anEntityManager, PhysicsWorld& aPhysicsWorld)
	: myEntityManager(anEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
	FW_MessageQueue& messageQueue = myEntityManager.GetMessageQueue();
	messageQueue.SubscribeToMessage<FW_PreEntityRemovedMessage>(std::bind(&LevelState::OnPreEntityRemoved, this, std::placeholders::_1));
	messageQueue.SubscribeToMessage<CollisionMessage>(std::bind(&LevelState::OnCollision, this, std::placeholders::_1));

	myCurrentLevelID = 0;
	LoadTiledLevel("level_one.tmx");
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
					myEntityManager.QueueRemovalAllEntities();
					myEntityManager.FlushEntityRemovals();
	
					LoadTiledLevel(myCurrentLevelInformation.myLevelName.GetBuffer());
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
		int width = parser.GetIntAttribute("width");
		int height = parser.GetIntAttribute("height");

		FW_String tilesetSource;
		int firstGID = 0;
		
		if (parser.BeginElement("properties"))
		{
			while (parser.BeginElement("property"))
			{
				FW_String propertyName = parser.GetStringAttribute("name");
				if (propertyName == "next_level")
					myCurrentLevelInformation.myNextLevelName = parser.GetStringAttribute("value");

				parser.EndElement();
			}
			parser.EndElement();
		}

		{
			FW_PROFILE_SCOPE("Tileset Loading");

			while (parser.BeginElement("tileset"))
			{
				FW_String tilesetPath;
				FW_FileSystem::RemoveFileName(levelPath, tilesetPath);

				tilesetSource = parser.GetStringAttribute("source");
				tilesetSource = tilesetPath + parser.GetStringAttribute("source");

				firstGID = parser.GetIntAttribute("firstgid");

				LoadTileSheet(tilesetSource.GetBuffer(), firstGID);

				parser.EndElement();
			}
		}

		while (parser.BeginElement("layer"))
		{
			if (parser.BeginElement("data"))
			{
				while (parser.BeginElement("chunk"))
				{
					int startX = parser.GetIntAttribute("x");
					int startY = parser.GetIntAttribute("y");

					int chunkWidth = parser.GetIntAttribute("width");
					int chunkHeight = parser.GetIntAttribute("height");

					FW_GrowingArray<int> tileIDs;
					for (int y = 0; y < chunkHeight; ++y)
					{
						parser.GetRawDataAsInt(tileIDs, ",");

						for(int x = 0; x < tileIDs.Count(); ++x)
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

					parser.EndElement();
				}

				parser.EndElement();
			}

			parser.EndElement();
		}

		parser.EndElement();
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
				parser.EndElement();
			}

			parser.EndElement();
		}

		parser.EndElement();
	}
}

FW_EntityID LevelState::CreateTile(const Vector2f& aPosition, const Tilesheet::TileData& someTileData)
{
	FW_EntityID tile = myEntityManager.CreateEmptyEntity();
	RenderComponent& render = myEntityManager.AddComponent<RenderComponent>(tile);
	render.myTextureFileName = someTileData.myTexturePath;
	render.myTexture = FW_Renderer::GetTexture(render.myTextureFileName.GetBuffer());;
	render.mySpriteSize = render.myTexture.mySize;
	render.myTextureRect = MakeRect<int>(0, 0, render.myTexture.mySize.x, render.myTexture.mySize.y);

	TranslationComponent& translation = myEntityManager.AddComponent<TranslationComponent>(tile);
	translation.myPosition = aPosition;

	PhysicsComponent& physics = myEntityManager.AddComponent<PhysicsComponent>(tile);
	if (someTileData.myIsSpawnPoint) // This essentially means 'IsPlayer' atm, but the player should probably be created separately from this later
		physics.myObject = new PhysicsObject(new CircleShape(render.mySpriteSize.x * 0.5f));
	else
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

	if (someTileData.myIsSpawnPoint)
	{
		physics.myObject->myRestitution = 0.f;
		physics.myObject->SetDensity(25.f);
		physics.myObject->SetInertia(0.f);

		myEntityManager.AddComponent<PlayerComponent>(tile);
	}

	myPhysicsWorld.AddObject(physics.myObject);

	return tile;
}