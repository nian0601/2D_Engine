#include "PhysicsWorld.h"
#include <FW_Time.h>
#include <FW_Vector2.h>
#include <FW_Math.h>
#include <FW_Renderer.h>

PhysicsObject::PhysicsObject(PhysicsShape* aShape)
{
	myShape = aShape;
	myShape->myObject = this;
	aShape->SetOrientation(myOrientation);
	aShape->ComputeMass(1.f);
}

PhysicsObject::~PhysicsObject()
{
	delete myShape;
}

void PhysicsObject::IntegrateForces(float aDelta)
{
	if (myInvMass == 0.f)
		return;

	const Vector2f gravity = PhysicsWorld::ourGravity * PhysicsWorld::ourGravityScale;
	myVelocity += (myForces * myInvMass + gravity) * aDelta;
	myAngularVelocity += myTorque * myInvInertia * aDelta;
}

void PhysicsObject::IntegrateVelocity(float aDelta)
{
	myPosition += myVelocity * aDelta;
	myOrientation += myAngularVelocity * aDelta;
	SetOrientation(myOrientation);
	IntegrateForces(aDelta);
}

void PhysicsObject::ApplyImpulse(const Vector2f& aImpulse, const Vector2f& aContactVector)
{
	myVelocity += myInvMass * aImpulse;
	myAngularVelocity += myInvInertia * Cross(aContactVector, aImpulse);
}

void PhysicsObject::SetMass(float aMass)
{
	myMass = aMass;

	if (myMass > 0.f)
		myInvMass = 1.f / aMass;
	else
		myInvMass = 0.f;
}

void PhysicsObject::SetInertia(float aIntertia)
{
	myInertia = aIntertia;
	if (myInertia > 0.f)
		myInvInertia = 1.f / myInertia;
	else
		myInvInertia = 0.f;
}

void PhysicsObject::SetDensity(float aDensity)
{
	myShape->ComputeMass(aDensity);
}

void PhysicsObject::MakeStatic()
{
	myMass = 0.f;
	myInvMass = 0.f;
	myInertia = 0.f;
	myInvInertia = 0.f;
}

void PhysicsObject::MakeSensor()
{
	mySensorFlag = true;
}

void PhysicsObject::SetPosition(const Vector2f& aPosition)
{
	myPosition = aPosition;
	myPreviousPosition = aPosition;
}

void PhysicsObject::SetOrientation(float aRadians)
{
	myOrientation = aRadians;

	if (myOrientation > FW_PI * 2.f)
		myOrientation = 0.f;
	if (myOrientation < -FW_PI * 2.f)
		myOrientation = 0.f;

	myPreviousOrientation = myOrientation;
	myShape->SetOrientation(myOrientation);
}

//////////////////////////////////////////////////////////////////////////

const Vector2f PhysicsWorld::ourGravity = Vector2f(0.f, 9.82f);
float PhysicsWorld::ourGravityScale = 7.f;

PhysicsWorld::PhysicsWorld()
	: myFixedDeltaTime(1.f / 60.f)
{
	myCircleShape = new CircleShape(5.f);
	myCircleObject = new PhysicsObject(myCircleShape);
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
		PhysicsObject* A = myObjects[i];
		A->myPreviousPosition = A->myPosition;
		A->myPreviousVelocity = A->myVelocity;
		A->myPreviousOrientation = A->myOrientation;

		for (int j = i + 1; j < myObjects.Count(); ++j)
		{
			const PhysicsObject* B = myObjects[j];

			if(A->myInvMass == 0.f && B->myInvMass == 0.f)
				continue;

			if (myObjects[i]->myShape->RunCollision(*myObjects[j]->myShape, manifold))
				myContacts.Add(manifold);
		}
	}

	for (PhysicsObject* object : myObjects)
		object->IntegrateForces(myFixedDeltaTime);

	const int maxNumIterations = 10;
	for (int i = 0; i < maxNumIterations; ++i)
	{
		for (const Manifold& manifold : myContacts)
			ResolveCollision(manifold);

		//for (MaxDistanceConstraint* constraint : myMaxDistanceConstraints)
		//	constraint->ResolveConstraint();
	}

	for (PhysicsObject* object : myObjects)
		object->IntegrateVelocity(myFixedDeltaTime);

	for (const Manifold& manifold : myContacts)
		PositionalCorrection(manifold);

	for (PhysicsObject* object : myObjects)
	{
		object->myForces = { 0.f, 0.f };
		object->myTorque = 0.f;
	}
}

void PhysicsWorld::TickLimited(float aDeltaTime)
{
	static float accumulator = 0.f;

	accumulator += aDeltaTime;
	accumulator = FW_Clamp(accumulator, 0.f, 1.f);

	//const bool myIsInSingleStepMode = false;
	//const bool myShouldStepSingleFrame = false;

	while (accumulator >= GetFixedDeltaTime())
	{
		//if (myIsInSingleStepMode)
		//{
		//	if (myShouldStepSingleFrame)
		//	{
		//		Tick();
		//		myShouldStepSingleFrame = false;
		//	}
		//}
		//else
		{
			Tick();
		}

		accumulator -= GetFixedDeltaTime();
	}
}

void PhysicsWorld::RenderAllObjects()
{
	for (const PhysicsObject* object : GetObjects())
		object->myShape->Render();
}

void PhysicsWorld::RenderContacts()
{
	for (const Manifold& contact : myContacts)
	{
		for (int i = 0; i < contact.myContactCount; ++i)
		{
			FW_Renderer::RenderLine(contact.myContacts[i], contact.myContacts[i] + contact.myHitNormal * contact.myPenetrationDepth, 0xFFFF0000);
			FW_Renderer::RenderCircle(contact.myContacts[i], 3.f);
		}
	}
}

void PhysicsWorld::ApplyForceInRadius(const Vector2f& aCenter, float aRadius, float aMinForce, float aMaxForce)
{
	myCircleObject->myPosition = aCenter;
	myCircleShape->myRadius = aRadius;

	Manifold manifold;
	for (PhysicsObject* object : myObjects)
	{
		if (object->myShape->RunCollision(*myCircleShape, manifold))
		{
			float forceScale = FW_Clamp(abs(manifold.myPenetrationDepth), 0.f, myCircleShape->myRadius);
			forceScale /= myCircleShape->myRadius;

			float finalForce = FW_Lerp(aMinForce, aMaxForce, forceScale);
			object->myVelocity += object->myInvMass * manifold.myHitNormal * finalForce;
		}
	}
}

void PhysicsWorld::DeleteAllObjects()
{
	myMaxDistanceConstraints.DeleteAll();
	myObjects.DeleteAll();
}

void PhysicsWorld::AddObject(PhysicsObject* aObject)
{
	myObjects.Add(aObject);
}

void PhysicsWorld::AddConstraint(MaxDistanceConstraint* aConstraint)
{
	myMaxDistanceConstraints.Add(aConstraint);
}

void PhysicsWorld::RemoveObject(PhysicsObject* aObject)
{
	myObjects.RemoveCyclic(aObject);
}

void PhysicsWorld::ResolveCollision(const Manifold& aManifold)
{
	PhysicsObject& A = *aManifold.myObjectA;
	PhysicsObject& B = *aManifold.myObjectB;

	if (A.myInvMass + B.myInvMass == 0.f)
	{
		A.myVelocity = { 0.f, 0.f };
		B.myVelocity = { 0.f, 0.f };
		return;
	}

	if (A.mySensorFlag || B.mySensorFlag)
		return;

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
	PhysicsObject& A = *aManifold.myObjectA;
	PhysicsObject& B = *aManifold.myObjectB;

	if (A.mySensorFlag || B.mySensorFlag)
		return;

	const float slop = 0.05f;
	const float percent = 0.3f;
	Vector2f correction = (FW_Max(aManifold.myPenetrationDepth - slop, 0.f) / (A.myInvMass + B.myInvMass)) * percent * aManifold.myHitNormal;

	A.myPosition -= A.myInvMass * correction;
	B.myPosition += B.myInvMass * correction;
}

void MaxDistanceConstraint::ResolveConstraint()
{
	PhysicsObject& A = *myObjectA;
	PhysicsObject& B = *myObjectB;

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