#pragma once
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>

class PhysicsWorld;
struct Object;
struct MaxDistanceConstraint;

class ChainBuilder
{
public:
	ChainBuilder(PhysicsWorld& aPhysicsWorld);
	~ChainBuilder();

	bool Update(const Vector2f& aMousePosition);

private:
	PhysicsWorld& myPhysicsWorld;
	FW_GrowingArray<Object*> myObjects;
	FW_GrowingArray<MaxDistanceConstraint*> myConstraints;

	float myAnchorSize;
	float myLinkSize;
	bool myLastObjectWasAnAnchor;
};
