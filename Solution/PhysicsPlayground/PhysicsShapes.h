#pragma once
#include <FW_Rect.h>
#include <FW_Matrix22.h>

struct CircleShape;
struct AABBShape;
struct PolygonShape;

struct Manifold;
struct Object;

struct Shape
{
	virtual ~Shape() {}
	virtual bool RunCollision(const Shape& aShape, Manifold& aManifold) const = 0;

	virtual bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const = 0;
	virtual bool TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const = 0;
	virtual bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const = 0;

	virtual void Render() const = 0;

	virtual void SetOrientation(float aRadians) { aRadians; }
	virtual void ComputeMass(float aDensity) { aDensity; }

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
	bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const override;

	void Render() const override;

	void ComputeMass(float aDensity) override;
};

struct PolygonShape : public Shape
{
	PolygonShape(const Vector2f& aSize);

	bool RunCollision(const Shape& aShape, Manifold& aManifold) const;
	bool TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const override;
	bool TestCollision(const AABBShape& aAABBShape, Manifold& aManifold) const override;
	bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const override;

	void Render() const override;

	void SetOrientation(float aRadians) override;
	void ComputeMass(float aDensity) override;

	void MakeBox(const Vector2f& aSize);

	Vector2f GetSupport(const Vector2f& aDirection) const;

	static const int MaxVertexCount = 64;

	int myVertexCount;
	Vector2f myVertices[MaxVertexCount];
	Vector2f myNormals[MaxVertexCount];

	FW_Matrix22 myModelSpace;
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
	bool TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const override;

	void Render() const override;
};