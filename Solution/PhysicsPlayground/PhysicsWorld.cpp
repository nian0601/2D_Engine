#include "PhysicsWorld.h"
#include <FW_Time.h>
#include <FW_Vector2.h>
#include <FW_Math.h>
#include <FW_Renderer.h>

Object::Object(Shape* aShape)
{
	myShape = aShape;
	myShape->myObject = this;
	aShape->SetOrientation(myOrientation);
	aShape->ComputeMass(1.f);
}

Object::~Object()
{
	delete myShape;
}

void Object::IntegrateForces(float aDelta)
{
	if (myInvMass == 0.f)
		return;

	const Vector2f gravity = PhysicsWorld::ourGravity * PhysicsWorld::ourGravityScale;
	myVelocity += (myForces * myInvMass + gravity) * aDelta;
	myAngularVelocity += myTorque * myInvInertia * aDelta;
}

void Object::IntegrateVelocity(float aDelta)
{
	myPosition += myVelocity * aDelta;
	myOrientation += myAngularVelocity * aDelta;
	SetOrientation(myOrientation);
	IntegrateForces(aDelta);
}

void Object::ApplyImpulse(const Vector2f& aImpulse, const Vector2f& aContactVector)
{
	myVelocity += myInvMass * aImpulse;
	myAngularVelocity += myInvInertia * Cross(aContactVector, aImpulse);
}

void Object::SetMass(float aMass)
{
	myMass = aMass;

	if (myMass > 0.f)
		myInvMass = 1.f / aMass;
	else
		myInvMass = 0.f;
}

void Object::SetInertia(float aIntertia)
{
	myInertia = aIntertia;
	if (myInertia > 0.f)
		myInvInertia = 1.f / myInertia;
	else
		myInvInertia = 0.f;
}

void Object::SetStatic()
{
	myMass = 0.f;
	myInvMass = 0.f;
	myInertia = 0.f;
	myInvInertia = 0.f;
}

void Object::SetOrientation(float aRadians)
{
	myOrientation = aRadians;
	myShape->SetOrientation(aRadians);
}

//////////////////////////////////////////////////////////////////////////

const Vector2f PhysicsWorld::ourGravity = Vector2f(0.f, 9.82f);
float PhysicsWorld::ourGravityScale = 7.f;

PhysicsWorld::PhysicsWorld()
	: myFixedDeltaTime(1.f / 60.f)
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
	Manifold manifold;
	myContacts.RemoveAll();

	for (int i = 0; i < myObjects.Count(); ++i)
	{
		Object* A = myObjects[i];
		A->myPreviousPosition = A->myPosition;
		A->myPreviousOrientation = A->myOrientation;

		for (int j = i + 1; j < myObjects.Count(); ++j)
		{
			const Object* B = myObjects[j];

			if(A->myInvMass == 0.f && B->myInvMass == 0.f)
				continue;

			if (myObjects[i]->myShape->RunCollision(*myObjects[j]->myShape, manifold))
				myContacts.Add(manifold);
		}
	}

	for (Object* object : myObjects)
		object->IntegrateForces(myFixedDeltaTime);

	const int maxNumIterations = 10;
	for (int i = 0; i < maxNumIterations; ++i)
	{
		for (const Manifold& manifold : myContacts)
			ResolveCollision(manifold);

		//for (MaxDistanceConstraint* constraint : myMaxDistanceConstraints)
		//	constraint->ResolveConstraint();
	}

	for (Object* object : myObjects)
		object->IntegrateVelocity(myFixedDeltaTime);

	for (const Manifold& manifold : myContacts)
		PositionalCorrection(manifold);

	for (Object* object : myObjects)
	{
		object->myForces = { 0.f, 0.f };
		object->myTorque = 0.f;
	}
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

	float e = FW_Min(A.myRestitution, B.myRestitution);
	float sf = A.myStaticFriction * A.myStaticFriction;
	float df = A.myDynamicFriction * A.myDynamicFriction;

	for (int i = 0; i < aManifold.myContactCount; ++i)
	{
		Vector2f ra = aManifold.myContacts[i] - A.myPosition;
		Vector2f rb = aManifold.myContacts[i] - B.myPosition;

		Vector2f rv = B.myVelocity + Cross(B.myAngularVelocity, rb) - A.myVelocity - Cross(A.myAngularVelocity, ra);

		float contactVelocity = Dot(rv, aManifold.myHitNormal);
		if (contactVelocity > 0.f)
			return;

		float raCrossN = Cross(ra, aManifold.myHitNormal);
		float rbCrossN = Cross(rb, aManifold.myHitNormal);
		float invMassSum = A.myInvMass + B.myInvMass + FW_Square(raCrossN) * A.myInvInertia + FW_Square(rbCrossN) * B.myInvInertia;

		// Impulse-scalar
		float j = -(1.f + e) * contactVelocity;
		j /= invMassSum;
		j /= static_cast<float>(aManifold.myContactCount);

		Vector2f impulse = aManifold.myHitNormal * j;
		A.ApplyImpulse(-impulse, ra);
		B.ApplyImpulse( impulse, rb);

		// Friction
		rv = B.myVelocity + Cross(B.myAngularVelocity, rb) - A.myVelocity - Cross(A.myAngularVelocity, ra);
		Vector2f t = rv - (aManifold.myHitNormal * Dot(rv, aManifold.myHitNormal));
		Normalize(t);
		
		float jT = -Dot(rv, t);
		jT /= invMassSum;
		jT /= static_cast<float>(aManifold.myContactCount);
		
		if (FW_Equal(jT, 0.f))
			return;
		
		Vector2f tangentImpulse;
		if (abs(jT) < j * sf)
			tangentImpulse = t * jT;
		else
			tangentImpulse = t * -j * df;
		
		A.ApplyImpulse(-tangentImpulse, ra);
		B.ApplyImpulse( tangentImpulse, rb);
	}
}

void PhysicsWorld::PositionalCorrection(const Manifold& aManifold)
{
	Object& A = *aManifold.myObjectA;
	Object& B = *aManifold.myObjectB;

	const float slop = 0.05f;
	const float percent = 0.3f;
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