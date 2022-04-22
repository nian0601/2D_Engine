#pragma once

#include "FW_IGame.h"

#include <FW_Vector2.h>
#include <FW_GrowingArray.h>
#include <FW_Rect.h>
#include "PhysicsWorld.h"

class ChainBuilder;
class PhysicsPlayground : public FW_IGame
{
public:
	void OnStartup() override;
	void OnShutdown() override;
	bool Run() override;

	void BuildGameImguiEditor(unsigned int aGameOffscreenBufferTextureID) override;


	const char* GetGameName() override { return "PhysicsPlayground"; }
	const char* GetDataFolderName() override { return "PhysicsPlayground"; }

private:
	enum SceneType
	{
		RandomCircles,
		ChainTest,
		BoxesAndCircles,
		Polygon,
	};
	int mySelectedSceneType;

	void GenerateScene(SceneType aSceneType);
	void GenerateRandomCirclesScene();
	void GenerateChainTestScene();
	void GenerateBoxesAndCirclesScene();
	void GeneratePolygonScene();

	PhysicsWorld myPhysicsWorld;
	Object* myPlayerObject;

	ChainBuilder* myChainBuilder;

	bool myIsInSingleFrameMode;
	bool myShouldStepSingleFrame;
	int myNumberOfCircles;
	Vector2i myMinMaxRadius;
	Vector2f myMinMaxRestitution;

	bool myRenderContacts;
};
