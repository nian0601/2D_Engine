#pragma once

#include "Structs.h"
#include "FW_GrowingArray.h"

struct Hitable
{
	Hitable() {}
	Hitable(const Material& aMaterial)
		: myMaterial(aMaterial)
	{}

	virtual bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const = 0;
	virtual void BuildAABB() = 0;

	AABB myAABB;

	Material myMaterial;
	FW_String myUIName;
};

struct Sphere : public Hitable
{
	Sphere() {}
	Sphere(const Vector3f& aPosition, float aRadius)
		: myPosition(aPosition), myRadius(aRadius) 
	{}

	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	Vector3f myPosition;
	float myRadius;
};

struct XY_Rect : public Hitable
{
	XY_Rect() {}
	XY_Rect(float _x0, float _x1, float _y0, float _y1, float _k, const Material& aMaterial)
		: Hitable(aMaterial), x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k)
	{}

	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	float x0, x1, y0, y1, k = 0.f;
};

struct XZ_Rect : public Hitable
{
	XZ_Rect() {}
	XZ_Rect(float _x0, float _x1, float _z0, float _z1, float _k, const Material& aMaterial)
		: Hitable(aMaterial), x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k)
	{}

	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	float x0, x1, z0, z1, k = 0.f;
};

struct YZ_Rect : public Hitable
{
	YZ_Rect() {}
	YZ_Rect(float _y0, float _y1, float _z0, float _z1, float _k, const Material& aMaterial)
		: Hitable(aMaterial), y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k)
	{}

	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	float y0, y1, z0, z1, k = 0.f;
};

struct Box : public Hitable
{
	Box() {}
	Box(const Vector3f& aMinPoint, const Vector3f& aMaxPoint, const Material& aMaterial);

	bool Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const override;
	void BuildAABB() override;

	Vector3f myMin;
	Vector3f myMax;
	FW_GrowingArray<const Hitable*> mySides;

	XY_Rect myXY0;
	XY_Rect myXY1;

	XZ_Rect myXZ0;
	XZ_Rect myXZ1;

	YZ_Rect myYZ0;
	YZ_Rect myYZ1;
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
	void AddObject(const XZ_Rect& aRect);
	void AddObject(const YZ_Rect& aRect);
	void AddObject(const Box& aBox);
	bool CastRay(const Ray& aRay, RayHit& aOutHit) const;

	void BuildOctree();
	void ClearOctree();

	void ClearWorld();

	FW_GrowingArray<Sphere>& GetAllSpheres() { return mySpheres; }

private:
	FW_GrowingArray<Sphere> mySpheres;
	FW_GrowingArray<XY_Rect> myXYRects;
	FW_GrowingArray<XZ_Rect> myXZRects;
	FW_GrowingArray<YZ_Rect> myYZRects;
	FW_GrowingArray<Box> myBoxes;
	Octtree myOctree;
};