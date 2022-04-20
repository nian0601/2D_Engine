#pragma once
#include <FW_Rect.h>
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>


// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493
// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032?_ga=2.10454710.2040535734.1649969427-1859424014.1649969427
// https://github.com/tutsplus/ImpulseEngine

// Goes over constraints in p3
// https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics

struct Shape;
struct Object
{
	Object(Shape* aShape);

	~Object();

	Vector2f myPosition; // += myVelocity * deltaTime
	Vector2f myVelocity; // += myAcceleration
	Vector2f myAcceleration; // myForces / myMass
	Vector2f myForces; // Cleared every frame

	float myRestitution = 1.0f; //Bouncyness, 0 == BeanBag, 1 == SuperBall
	float myMass = 1.f;
	float myInvMass = 1.f;
	float myStaticFriction = 0.5f;
	float myDynamicFriction = 0.25f;

	int myColor = 0xFFFFFFFF;

	Shape* myShape = nullptr;

	void SetMass(float aMass)
	{
		myMass = aMass;

		if (myMass > 0.f)
			myInvMass = 1.f / aMass;
		else
			myInvMass = 0.f;
	}
};

struct Manifold
{
	Object* myObjectA;
	Object* myObjectB;
	Vector2f myHitNormal;
	float myPenetrationDepth;
};

struct CircleShape;
struct AABBShape;
struct Shape
{
	virtual ~Shape() {}
	virtual bool RunCollision(const Shape& aShape, Manifold& aManifold) const = 0;

	virtual bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const = 0;
	virtual bool TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const = 0;

	virtual void Render() const = 0;

	Object* myObject = nullptr;
};

struct CircleShape : public Shape
{
	CircleShape(float aRadius)
		: myRadius(aRadius)
	{}

	float myRadius;

	bool RunCollision(const Shape& aShape, Manifold& aManifold) const;
	bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const override;
	bool TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const override;

	void Render() const override;
};

struct AABBShape : public Shape
{
	AABBShape(const Rectf& aRect)
		: myRect(aRect)
	{}

	Rectf myRect;

	bool RunCollision(const Shape& aShape, Manifold& aManifold) const;
	bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const override;
	bool TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const override;

	void Render() const override;
};

struct MaxDistanceConstraint
{
	Object* myObjectA;
	Object* myObjectB;
	float myMaxDistance;

	void ResolveConstraint();
};

class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	void Tick();

	void ApplyForceInRadius(const Vector2f& aCenter, float aRadius, float aMinForce, float aMaxForce);

	void DeleteAllObjects();

	void AddObject(Object* aObject);
	void AddConstraint(MaxDistanceConstraint* aConstraint);

	const FW_GrowingArray<Object*>& GetObjects() const { return myObjects; }

	float GetFixedDeltaTime() const { return myFixedDeltaTime; }
	float myGravityScale;

private:
	void ResolveCollision(const Manifold& aManifold);
	void PositionalCorrection(const Manifold& aManifold);

	float myFixedDeltaTime;
	FW_GrowingArray<Object*> myObjects;
	FW_GrowingArray<MaxDistanceConstraint*> myMaxDistanceConstraints;

	Object* myCircleObject;
	CircleShape* myCircleShape;
};
