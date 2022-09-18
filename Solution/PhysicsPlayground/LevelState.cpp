#include "LevelState.h"

#include <FW_EntityManager.h>
#include <FW_MessageQueue.h>
#include <FW_XMLParser.h>

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
	//LoadLevel(myCurrentLevelID);

	//LoadTileSheet("blue_tiles.tsx");
	//LoadTiledLevel("PhysicsPlayground/Data/level_one.tmx");
	LoadTiledLevel("Levels/test_level.tmx");
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

void LevelState::LoadTiledLevel(const char* aFilePath)
{
	FW_XMLParser parser(aFilePath);

	if (parser.BeginElement("map"))
	{
		int width = parser.GetIntAttribute("width");
		int height = parser.GetIntAttribute("height");

		FW_String tilesetSource;
		int firstGID = 0;
		
		while (parser.BeginElement("tileset"))
		{
			FW_String tilesetPath;
			FW_FileSystem::RemoveFileName(aFilePath, tilesetPath);
			
			tilesetSource = parser.GetStringAttribute("source");
			tilesetSource = tilesetPath + parser.GetStringAttribute("source");

			firstGID = parser.GetIntAttribute("firstgid");

			LoadTileSheet(tilesetSource.GetBuffer(), firstGID);

			parser.EndElement();
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
								for (Tilesheet::TileData& tile : myTileSheet.myTiles)
								{
									if (tile.myID == tileIDs[x])
									{
										Vector2f position;
										position.x = (startX + x) * 64.f;
										position.y = (startY + y) * 64.f;

										FW_Renderer::Texture texture = FW_Renderer::GetTexture(tile.myTexturePath.GetBuffer());
										CreateTile(position, texture, tile.myTexturePath.GetBuffer());
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
			Tilesheet::TileData& tile = myTileSheet.myTiles.Add();
			tile.myID = parser.GetIntAttribute("id") + aFirstTileID;

			if (parser.BeginElement("properties"))
			{
				while (parser.BeginElement("property"))
				{
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