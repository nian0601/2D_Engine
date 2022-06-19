#pragma once
#include <FW_Rect.h>
#include <FW_Matrix22.h>

struct CircleShape;
struct PolygonShape;

struct Manifold;
struct PhysicsObject;

struct PhysicsShape
{
	virtual ~PhysicsShape() {}
	virtual bool RunCollision(const PhysicsShape& aShape, Manifold& aManifold) const = 0;

	virtual bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const = 0;
	virtual bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const = 0;

	virtual void Render() const = 0;

	virtual void SetOrientation(float aRadians) { aRadians; }
	virtual void ComputeMass(float aDensity) { aDensity; }

	PhysicsObject* myObject = nullptr;
};

struct CircleShape : public PhysicsShape
{
	CircleShape(float aRadius)
		: myRadius(aRadius)
	{}

	float myRadius;

	bool RunCollision(const PhysicsShape& aShape, Manifold& aManifold) const;
	bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const override;
	bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const override;

	void Render() const override;

	void ComputeMass(float aDensity) override;
};

struct PolygonShape : public PhysicsShape
{
	PolygonShape() {}

	bool RunCollision(const PhysicsShape& aShape, Manifold& aManifold) const;
	bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const override;
	bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const override;

	void Render() const override;

	void SetOrientation(float aRadians) override;
	void ComputeMass(float aDensity) override;

	Vector2f GetSupport(const Vector2f& aDirection) const;

	static const int MaxVertexCount = 64;

	int myVertexCount;
	Vector2f myVertices[MaxVertexCount];
	Vector2f myNormals[MaxVertexCount];

	FW_Matrix22 myModelSpace;
};

struct AABBShape : public PolygonShape
{
	AABBShape(const Vector2f& aSize);
	AABBShape(const Vector2i& aSize);

	void Render() const override;

private:
	// Mutable so that we can set the position of the rect when rendering
	mutable Rectf myRect;
};