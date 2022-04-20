#include "PhysicsWorld.h"
#include <FW_Time.h>
#include <FW_Vector2.h>
#include <FW_Math.h>
#include <FW_Renderer.h>



Object::Object(Shape* aShape)
{
	myShape = aShape;
	myShape->myObject = this;
}

Object::~Object()
{
	delete myShape;
}

bool CircleShape::RunCollision(const Shape& aShape, Manifold& aManifold) const
{
	return aShape.TestCollision(*this, aManifold);
}

bool CircleShape::TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const
{
	Vector2f collisionVector = aCircleShape.myObject->myPosition - myObject->myPosition;

	const float distance = Length(collisionVector);
	if (distance < myRadius + aCircleShape.myRadius)
	{
		aManifold.myObjectA = myObject;
		aManifold.myObjectB = aCircleShape.myObject;
		aManifold.myHitNormal = GetNormalized(collisionVector);
		aManifold.myPenetrationDepth = distance - (myRadius + aCircleShape.myRadius);
		return true;
	}

	return false;
}

bool CircleShape::TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const
{
	Vector2f n = aAABBShape.myObject->myPosition - myObject->myPosition;
	Vector2f closest = n;

	float halfXExtent = aAABBShape.myRect.myExtents.x * 0.5f;
	float halfYExtent = aAABBShape.myRect.myExtents.y * 0.5f;

	closest.x = FW_Clamp(closest.x, -halfXExtent, halfXExtent);
	closest.y = FW_Clamp(closest.y, -halfYExtent, halfYExtent);

	bool inside = false;
	if (n == closest)
	{
		inside = true;

		if (abs(n.x) > abs(n.y))
		{
			if (closest.x > 0.f)
				closest.x = halfXExtent;
			else
				closest.x = -halfXExtent;
		}
		else
		{
			if (closest.y > 0.f)
				closest.y = halfYExtent;
			else
				closest.y = -halfYExtent;
		}
	}

	Vector2f normal = n - closest;
	float distance = Length2(normal);
	float radius = myRadius;

	if (distance > radius * radius && !inside)
		return false;

	distance = sqrt(distance);
	Normalize(normal);

	if (inside)
	{
		aManifold.myHitNormal = -normal;
		aManifold.myPenetrationDepth = radius - distance;
	}
	else
	{
		aManifold.myHitNormal = normal;
		aManifold.myPenetrationDepth = radius - distance;
	}

	aManifold.myObjectA = myObject;
	aManifold.myObjectB = aAABBShape.myObject;
	return true;
}

void CircleShape::Render() const
{
	FW_Renderer::RenderCircle(myObject->myPosition, myRadius, myObject->myColor);
}

bool AABBShape::RunCollision(const Shape& aShape, Manifold& aManifold) const
{
	return aShape.TestCollision(*this, aManifold);
}

bool AABBShape::TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const
{
	Vector2f n = aCircleShape.myObject->myPosition - myObject->myPosition;
	Vector2f closest = n;

	float halfXExtent = myRect.myExtents.x * 0.5f;
	float halfYExtent = myRect.myExtents.y * 0.5f;

	closest.x = FW_Clamp(closest.x, -halfXExtent, halfXExtent);
	closest.y = FW_Clamp(closest.y, -halfYExtent, halfYExtent);

	bool inside = false;
	if (n == closest)
	{
		inside = true;

		if (abs(n.x) > abs(n.y))
		{
			if (closest.x > 0.f)
				closest.x = halfXExtent;
			else
				closest.x = -halfXExtent;
		}
		else
		{
			if (closest.y > 0.f)
				closest.y = halfYExtent;
			else
				closest.y = -halfYExtent;
		}
	}

	Vector2f normal = n - closest;
	float distance = Length2(normal);
	float radius = aCircleShape.myRadius;

	if (distance > radius * radius && !inside)
		return false;

	distance = sqrt(distance);
	Normalize(normal);

	if (inside)
	{
		aManifold.myHitNormal = -normal;
		aManifold.myPenetrationDepth = radius - distance;
	}
	else
	{
		aManifold.myHitNormal = normal;
		aManifold.myPenetrationDepth = radius - distance;
	}

	aManifold.myObjectA = myObject;
	aManifold.myObjectB = aCircleShape.myObject;
	return true;
}

bool AABBShape::TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const
{
	Object* A = myObject;
	Object* B = aAABBShape.myObject;

	Vector2f n = B->myPosition - A->myPosition;

	Rectf abox = myRect;
	Rectf bbox = aAABBShape.myRect;

	float a_extent_x = abox.myExtents.x * 0.5f;
	float b_extent_x = bbox.myExtents.x * 0.5f;

	float x_overlap = a_extent_x + b_extent_x - abs(n.x);

	if (x_overlap > 0.f)
	{
		float a_extent_y = abox.myExtents.y * 0.5f;
		float b_extent_y = bbox.myExtents.y * 0.5f;

		float y_overlap = a_extent_y + b_extent_y - abs(n.y);

		if (y_overlap > 0.f)
		{
			if (n.x < 0.f)
				aManifold.myHitNormal = { -1.f, 0.f };
			else
				aManifold.myHitNormal = { 1.f, 0.f }; // Should this be {1.f, 0.f}?

			aManifold.myObjectA = myObject;
			aManifold.myObjectB = aAABBShape.myObject;
			aManifold.myPenetrationDepth = x_overlap;
			return true;
		}
		else
		{
			if (n.y < 0.f)
				aManifold.myHitNormal = { 0.f, -1.f };
			else
				aManifold.myHitNormal = { 0.f, 1.f };

			aManifold.myObjectA = myObject;
			aManifold.myObjectB = aAABBShape.myObject;
			aManifold.myPenetrationDepth = y_overlap;
			return true;
		}
	}

	return false;
}

void AABBShape::Render() const
{
	FW_Renderer::RenderRect(myRect, myObject->myColor);
}

//////////////////////////////////////////////////////////////////////////

PhysicsWorld::PhysicsWorld()
	: myGravityScale(7.f)
	, myFixedDeltaTime(1.f / 60.f)
{
	myCircleShape = new CircleShape(5.f);
	myCircleObject = new Object(myCircleShape);
}


PhysicsWorld::~PhysicsWorld()
{
	delete myCircleObject;
	DeleteAllObjects();
}

void PhysicsWorld::Tick()
{
	const Vector2f gravity = { 0.f, 9.82f * myGravityScale };

	FW_GrowingArray<Manifold> manifolds;
	Manifold manifold;

	for (int i = 0; i < myObjects.Count(); ++i)
	{
		for (int j = i + 1; j < myObjects.Count(); ++j)
		{
			if (myObjects[i]->myShape->RunCollision(*myObjects[j]->myShape, manifold))
				manifolds.Add(manifold);
		}
	}

	for (Object* object : myObjects)
	{
		if(object->myInvMass == 0.f)
			continue;

		object->myVelocity += (object->myForces * object->myInvMass + gravity) * myFixedDeltaTime;
	}

	const int maxNumIterations = 10;
	for (int i = 0; i < maxNumIterations; ++i)
	{
		for (const Manifold& manifold : manifolds)
			ResolveCollision(manifold);

		for (MaxDistanceConstraint* constraint : myMaxDistanceConstraints)
			constraint->ResolveConstraint();
	}

	for (Object* object : myObjects)
		object->myPosition += object->myVelocity * myFixedDeltaTime;

	for (const Manifold& manifold : manifolds)
		PositionalCorrection(manifold);

	for (Object* object : myObjects)
		object->myForces = { 0.f, 0.f };
}

void PhysicsWorld::ApplyForceInRadius(const Vector2f& aCenter, float aRadius, float aMinForce, float aMaxForce)
{
	myCircleObject->myPosition = aCenter;
	myCircleShape->myRadius = aRadius;

	Manifold manifold;
	for (Object* object : myObjects)
	{
		if (object->myShape->RunCollision(*myCircleShape, manifold))
		{
			float forceScale = FW_Clamp(abs(manifold.myPenetrationDepth), 0.f, myCircleShape->myRadius);
			forceScale /= myCircleShape->myRadius;

			float finalForce = FW_Lerp(aMinForce, aMaxForce, forceScale);
			
			object->myForces += manifold.myHitNormal * finalForce;
		}
	}
}

void PhysicsWorld::DeleteAllObjects()
{
	myMaxDistanceConstraints.DeleteAll();
	myObjects.DeleteAll();
}

void PhysicsWorld::AddObject(Object* aObject)
{
	myObjects.Add(aObject);
}

void PhysicsWorld::AddConstraint(MaxDistanceConstraint* aConstraint)
{
	myMaxDistanceConstraints.Add(aConstraint);
}

void PhysicsWorld::ResolveCollision(const Manifold& aManifold)
{
	Object& A = *aManifold.myObjectA;
	Object& B = *aManifold.myObjectB;

	if (A.myInvMass + B.myInvMass == 0.f)
	{
		A.myVelocity = { 0.f, 0.f };
		B.myVelocity = { 0.f, 0.f };
		return;
	}

	Vector2f rv = B.myVelocity - A.myVelocity;
	float velAlongNormal = Dot(rv, aManifold.myHitNormal);

	if (velAlongNormal > 0.f)
		return;

	float e = FW_Min(A.myRestitution, B.myRestitution);

	float j = -(1.f + e) * velAlongNormal;
	j /= A.myInvMass + B.myInvMass;

	Vector2f impulse = j * aManifold.myHitNormal;
	A.myVelocity -= A.myInvMass * impulse;
	B.myVelocity += B.myInvMass * impulse;

	//
	// Friction
	//

	rv = B.myVelocity - A.myVelocity;
	Vector2f tangent = rv - Dot(rv, aManifold.myHitNormal) * aManifold.myHitNormal;
	Normalize(tangent);

	float jT = -Dot(rv, tangent);
	jT = jT / (A.myInvMass + B.myInvMass);

	if (jT == 0.f)
		return;

	float staticFriction = A.myStaticFriction * A.myStaticFriction;
	float dynamicFriction = A.myDynamicFriction * A.myDynamicFriction;

	Vector2f frictionImpulse;
	if (abs(jT) < j * staticFriction)
		frictionImpulse = jT * tangent;
	else
		frictionImpulse = -j * tangent * dynamicFriction;

	A.myVelocity -= A.myInvMass * frictionImpulse;
	B.myVelocity += B.myInvMass * frictionImpulse;
}

void PhysicsWorld::PositionalCorrection(const Manifold& aManifold)
{
	Object& A = *aManifold.myObjectA;
	Object& B = *aManifold.myObjectB;

	if (A.myInvMass + B.myInvMass == 0.f)
		return;

	const float percent = 0.5f;
	const float slop = 0.1f;
	Vector2f correction = (FW_Max(aManifold.myPenetrationDepth - slop, 0.f) / (A.myInvMass + B.myInvMass)) * percent * aManifold.myHitNormal;

	A.myPosition -= A.myInvMass * correction;
	B.myPosition += B.myInvMass * correction;
}

void MaxDistanceConstraint::ResolveConstraint()
{
	Object& A = *myObjectA;
	Object& B = *myObjectB;

	if (A.myInvMass + B.myInvMass == 0.f)
	{
		A.myVelocity = { 0.f, 0.f };
		B.myVelocity = { 0.f, 0.f };
		return;
	}

	Vector2f rv = A.myVelocity - B.myVelocity;

	Vector2f normal = B.myPosition - A.myPosition;
	if (Length(normal) <= myMaxDistance)
		return;

	Normalize(normal);

	float velAlongNormal = Dot(rv, normal);

	if (velAlongNormal > 0.f)
		return;

	float j = -velAlongNormal * 2.f;
	j /= A.myInvMass + B.myInvMass;

	Vector2f impulse = j * normal;
	A.myVelocity += A.myInvMass * impulse;
	B.myVelocity -= B.myInvMass * impulse;
}
