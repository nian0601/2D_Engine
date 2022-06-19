#pragma once

#include <FW_Vector2.h>
#include "PhysicsWorld.h"

class ChainBuilder;
class PhysicsTestingTestbed
{
public:
	PhysicsTestingTestbed();
	~PhysicsTestingTestbed();

	void Run();

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
	PhysicsObject* myPlayerObject;

	ChainBuilder* myChainBuilder;

	bool myIsInSingleFrameMode;
	bool myShouldStepSingleFrame;
	int myNumberOfCircles;
	Vector2i myMinMaxRadius;
	Vector2f myMinMaxRestitution;

	bool myRenderContacts;
};
