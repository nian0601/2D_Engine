#pragma once

#include "Structs.h"
#include "FW_GrowingArray.h"

struct Hitable
{
	virtual bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const = 0;
	virtual void BuildAABB() = 0;

	AABB myAABB;

	Material myMaterial;
	FW_String myUIName;
};

struct Sphere : public Hitable
{
	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	Vector3f myPosition;
	float myRadius;
};

struct XY_Rect : public Hitable
{
	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	float x0, x1, y0, y1, k = 0.f;
};

class Octtree
{
public:
	Octtree();
	~Octtree();

	void AddHitable(const Hitable* aHitable);
	void Clear();

	bool CastRay(const Ray& aRay, RayHit& aOutHit) const;

private:
	struct Node
	{
		Node(int aDepth, const AABB& anAABB);
		~Node();

		AABB myAABB;
		int myDepth;
		FW_GrowingArray<const Hitable*> myObjects;
		FW_GrowingArray<Node*> myChildNodes;
		FW_GrowingArray<AABB> myChildAABBs;
	};

	bool CastRayVsNode(const Ray& aRay, Node* aNode, RayHit& aOutHit, bool& aHitAnything, float& aClosestSoFar) const;
	void InsertHitableIntoNode(const Hitable* aHitable, Node* aNode);

	Node* myRootNode;
	float myTotalSize = 256.f;
};

class CollisionWorld
{
public:
	void AddObject(const Sphere& aSphere);
	void AddObject(const XY_Rect& aRect);
	bool CastRay(const Ray& aRay, RayHit& aOutHit) const;

	void BuildOctree();
	void ClearOctree();

	void ClearWorld();

	FW_GrowingArray<Sphere>& GetAllSpheres() { return mySpheres; }

private:
	FW_GrowingArray<Sphere> mySpheres;
	FW_GrowingArray<XY_Rect> myXYRects;
	Octtree myOctree;
};