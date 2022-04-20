#include "ChainBuilder.h"
#include "PhysicsWorld.h"
#include <FW_Input.h>

ChainBuilder::ChainBuilder(PhysicsWorld& aPhysicsWorld)
	: myPhysicsWorld(aPhysicsWorld)
	, myAnchorSize(10.f)
	, myLinkSize(5.f)
	, myLastObjectWasAnAnchor(false)
{
}

ChainBuilder::~ChainBuilder()
{
	myConstraints.DeleteAll();
	myObjects.DeleteAll();
}

bool ChainBuilder::Update(const Vector2f& aMousePosition)
{
	if (FW_Input::WasMouseReleased(FW_Input::RIGHTMB))
	{
		for (Object* object : myObjects)
			myPhysicsWorld.AddObject(object);

		for (MaxDistanceConstraint* constraint : myConstraints)
			myPhysicsWorld.AddConstraint(constraint);

		myObjects.RemoveAll();
		myConstraints.RemoveAll();

		return false;
	}

	bool leftButton = FW_Input::WasMouseReleased(FW_Input::LEFTMB);
	bool middleButton = FW_Input::WasMouseReleased(FW_Input::MIDDLEMB);
	if (leftButton || middleButton)
	{
		Object* previousLink = nullptr;
		if (myObjects.Count() > 0)
			previousLink = myObjects.GetLast();

		
		bool placeAnchor = middleButton;
		float radius = placeAnchor ? myAnchorSize : myLinkSize;

		Object* newLink = new Object(new CircleShape(radius));
		myObjects.Add(newLink);

		Vector2f placementPosition = aMousePosition;
		if (previousLink)
		{
			Vector2f placementDirection = GetNormalized(aMousePosition - previousLink->myPosition);
			float placementDistance = myLastObjectWasAnAnchor ? myAnchorSize : myLinkSize;
			placementDistance += radius;

			placementPosition = previousLink->myPosition + placementDirection * placementDistance;

			MaxDistanceConstraint* constraint = new MaxDistanceConstraint();
			constraint->myObjectA = previousLink;
			constraint->myObjectB = newLink;
			constraint->myMaxDistance = placementDistance;
			myConstraints.Add(constraint);
		}

		newLink->myPosition = placementPosition;
		if (placeAnchor)
		{
			newLink->SetMass(0.f);
			newLink->myColor = 0xFF444444;
			myLastObjectWasAnAnchor = true;
		}
		else
		{
			newLink->SetMass(10.f);
			newLink->myRestitution = 0.7f;
			myLastObjectWasAnAnchor = false;
		}
	}

	for (const Object* object : myObjects)
		object->myShape->Render();

	return true;
}

