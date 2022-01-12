#pragma once

#include "Structs.h"
#include "FW_GrowingArray.h"

class Octtree
{
public:
	Octtree();
	~Octtree();

	void AddSphere(const Sphere* aSphere);
	void Clear();

	bool CastRay(const Ray& aRay, RayHit& aOutHit) const;

private:
	struct Node
	{
		Node(int aDepth, const AABB& anAABB);
		~Node();

		AABB myAABB;
		int myDepth;
		FW_GrowingArray<const Sphere*> myObjects;
		FW_GrowingArray<Node*> myChildNodes;
		FW_GrowingArray<AABB> myChildAABBs;
	};

	bool CastRayVsNode(const Ray& aRay, Node* aNode, RayHit& aOutHit, bool& aHitAnything, float& aClosestSoFar) const;
	void InsertSphereIntoNode(const Sphere* aSphere, Node* aNode);

	Node* myRootNode;
	float myTotalSize = 256.f;
};

class CollisionWorld
{
public:
	void AddSphere(const Sphere& aSphere);
	bool CastRay(const Ray& aRay, RayHit& aOutHit) const;

	void BuildOctree();
	void ClearOctree();

	void ClearWorld();


	FW_GrowingArray<Sphere>& GetAllSpheres() { return mySpheres; }

private:
	FW_GrowingArray<Sphere> mySpheres;
	Octtree myOctree;
};