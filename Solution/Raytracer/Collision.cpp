#include "Collision.h"
#include <float.h>

bool RayHitAABB(const Ray& anRay, const AABB& anAABB)
{
	static const char RIGHT = 0;
	static const char LEFT = 1;
	static const char MIDDLE = 2;

	const Vector3f normalizedDirection = GetNormalized(anRay.myDirection);

	bool inside = true;
	char quadrant[3];
	float candiatePlane[3];

	// These could be skipped if I had array-access to vector-elements
	float minB[3] = { anAABB.myMinPos.x, anAABB.myMinPos.x, anAABB.myMinPos.z };
	float maxB[3] = { anAABB.myMaxPos.x, anAABB.myMaxPos.x, anAABB.myMaxPos.z };
	float origin[3] = { anRay.myPosition.x, anRay.myPosition.y, anRay.myPosition.z };
	float dir[3] = { normalizedDirection.x, normalizedDirection.y, normalizedDirection.z };
	
	/* Find candidate planes; this loop can be avoided if
	rays cast all from the eye(assume perpsective view) */
	for (int i = 0; i < 3; ++i)
	{
		if (origin[i] < minB[i])
		{
			quadrant[i] = LEFT;
			candiatePlane[i] = minB[i];
			inside = false;
		}
		else if (origin[i] > maxB[i])
		{
			quadrant[i] = RIGHT;
			candiatePlane[i] = maxB[i];
			inside = false;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
	}

	/* Ray origin inside bounding box */
	if (inside)
	{
		// HitPosition = RayOrigin
		return true;
	}

	/* Calculate T distances to candidate planes */
	float maxT[3];
	for (int i = 0; i < 3; ++i)
	{
		if (quadrant[i] != MIDDLE && dir[i] != 0.f)
			maxT[i] = (candiatePlane[i] - origin[i]) / dir[i];
		else
			maxT[i] = -1.f;
	}

	/* Get largest of the maxT's for final choice of intersection */
	int whichPlane = 0;
	for (int i = 1; i < 3; ++i)
	{
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;
	}

	/* Check final candidate actually inside box */
	if (maxT[whichPlane] < 0.f)
		return false;

	float hitPosition[3];
	for (int i = 0; i < 3; ++i)
	{
		if (whichPlane != i)
		{
			hitPosition[i] = origin[i] + maxT[whichPlane] * dir[i];
			if (hitPosition[i] < minB[i] || hitPosition[i] > maxB[i])
				return false;
		}
		else
		{
			hitPosition[i] = candiatePlane[i];
		}
	}

	/* ray hits box */
	return true;
}

bool FirstAABBContainedBySecond(const AABB& aFirst, const AABB& aSecond)
{
	if (aFirst.myMinPos.x < aSecond.myMinPos.x) return false;
	if (aFirst.myMinPos.y < aSecond.myMinPos.y) return false;
	if (aFirst.myMinPos.z < aSecond.myMinPos.z) return false;

	if (aFirst.myMaxPos.x > aSecond.myMaxPos.x) return false;
	if (aFirst.myMaxPos.y > aSecond.myMaxPos.y) return false;
	if (aFirst.myMaxPos.z > aSecond.myMaxPos.z) return false;

	return true;
}

void SplitAABB(const AABB& anAABB, FW_GrowingArray<AABB>& outAABBs)
{
	const Vector3f halfExtents = anAABB.myExtents * 0.5f;

	Vector3f min = anAABB.myMinPos;
	Vector3f center = anAABB.myCenterPos;

	// Bottom Back Left (-x, -y, -z in unit-cube)
	AABB& bottomBackLeftAABB = outAABBs.Add();
	bottomBackLeftAABB = AABBFromMinCornerAndSize({ min.x, min.y, min.z }, halfExtents);

	// Bottom Back Right (x, -y, -z in unit-cube)
	AABB& bottomBackRightAABB = outAABBs.Add();
	bottomBackRightAABB = AABBFromMinCornerAndSize({ center.x, min.y, min.z }, halfExtents);

	// Top Back Left (-x, y, -z in unit-cube)
	AABB& topBackLeftAABB = outAABBs.Add();
	topBackLeftAABB = AABBFromMinCornerAndSize({ min.x, center.y, min.z }, halfExtents);

	// Top Back Right (x, y, -z in unit-cube)
	AABB& topBackRightAABB = outAABBs.Add();
	topBackRightAABB = AABBFromMinCornerAndSize({ center.x, center.y, min.z }, halfExtents);

	// Bottom Front Left (-x, -y, z in unit-cube)
	AABB& bottomFrontLeftAABB = outAABBs.Add();
	bottomFrontLeftAABB = AABBFromMinCornerAndSize({ min.x, min.y, center.z }, halfExtents);

	// Bottom Front Right (x, -y, z in unit-cube)
	AABB& bottomFrontRightAABB = outAABBs.Add();
	bottomFrontRightAABB = AABBFromMinCornerAndSize({ center.x, min.y, center.z }, halfExtents);

	// Top Front Left (-x, y, z in unit-cube)
	AABB& topFrontLeftAABB = outAABBs.Add();
	topFrontLeftAABB = AABBFromMinCornerAndSize({ min.x, center.y, center.z }, halfExtents);

	// Top Front Right (x, y, z in unit-cube)
	AABB& topFrontRightAABB = outAABBs.Add();
	topFrontRightAABB = AABBFromMinCornerAndSize({ center.x, center.y, center.z }, halfExtents);
}

//////////////////////////////////////////////////////////////////////////

bool Sphere::Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const
{
	Vector3f oc = aRay.myPosition - myPosition;
	float a = Length2(aRay.myDirection);
	float halfB = Dot(oc, aRay.myDirection);
	float c = Length2(oc) - myRadius * myRadius;
	float discriminant = halfB * halfB - a * c;

	if (discriminant < 0)
		return false;

	float squareDiscriminant = sqrt(discriminant);
	float root = (-halfB - squareDiscriminant) / a;
	if (root < aMinT || aMaxT < root)
	{
		root = (-halfB + squareDiscriminant) / a;
		if (root < aMinT || aMaxT < root)
			return false;
	}

	aHitRecord.myT = root;
	aHitRecord.myPosition = aRay.PositionAt(aHitRecord.myT);
	Vector3f outwardNormal = (aHitRecord.myPosition - myPosition) / myRadius;
	aHitRecord.SetFaceNormal(aRay, outwardNormal);
	aHitRecord.myMaterial = myMaterial;
	return true;
}


void Sphere::BuildAABB()
{
	const Vector3f sphereMin = myPosition - myRadius;
	const Vector3f sphereMax = myPosition + myRadius;
	myAABB = AABBFromPoints(sphereMin, sphereMax);
}

bool XY_Rect::Hit(const Ray& aRay, float aMinT, float aMaxT, RayHit& aHitRecord) const
{
	float t = (k - aRay.myPosition.z) / aRay.myDirection.z;
	if (t < aMinT || t > aMaxT)
		return false;

	float x = aRay.myPosition.x + t * aRay.myDirection.x;
	float y = aRay.myPosition.y + t * aRay.myDirection.y;
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;

	aHitRecord.myT = t;
	aHitRecord.myPosition = aRay.PositionAt(t);
	aHitRecord.SetFaceNormal(aRay, { 0.f, 0.f, 1.f });
	aHitRecord.myMaterial = myMaterial;
	return true;
}

void XY_Rect::BuildAABB()
{
	const Vector3f min(x0, y0, k - 0.0001f);
	const Vector3f max(x1, y1, k + 0.0001f);
	myAABB = AABBFromPoints(min, max);
}

//////////////////////////////////////////////////////////////////////////



Octtree::Node::Node(int aDepth, const AABB& anAABB)
	: myAABB(anAABB)
	, myDepth(aDepth)
{
	if (aDepth <= 0)
		return;

	FW_GrowingArray<AABB> childAABBs;
	SplitAABB(anAABB, myChildAABBs);

	for (int i = 0; i < 8; ++i)
		myChildNodes.Add(nullptr);
}

Octtree::Node::~Node()
{
	myChildNodes.DeleteAll();
}


Octtree::Octtree()
{
	myRootNode = new Node(4, AABBFromExtents({ 0.f, 0.f, 0.f }, { myTotalSize, myTotalSize, myTotalSize }));
}

Octtree::~Octtree()
{
	delete myRootNode;
}

void Octtree::AddHitable(const Hitable* aHitable)
{
	InsertHitableIntoNode(aHitable, myRootNode);
}

void Octtree::Clear()
{
	delete myRootNode;
	myRootNode = new Node(4, AABBFromExtents({ 0.f, 0.f, 0.f }, { myTotalSize, myTotalSize, myTotalSize }));
}

bool Octtree::CastRay(const Ray& aRay, RayHit& aOutHit) const
{
	bool hitAnything = false;
	float closestSoFar = FLT_MAX;

	return CastRayVsNode(aRay, myRootNode, aOutHit, hitAnything, closestSoFar);
}

bool Octtree::CastRayVsNode(const Ray& aRay, Node* aNode, RayHit& aOutHit, bool& aHitAnything, float& aClosestSoFar) const
{
	for (const Hitable* hitable : aNode->myObjects)
	{
		if (hitable->Hit(aRay, 0.01f, aClosestSoFar, aOutHit))
		{
			aHitAnything = true;
			aClosestSoFar = aOutHit.myT;
		}
	}

	for (Node* child : aNode->myChildNodes)
	{
		if (child && RayHitAABB(aRay, child->myAABB))
			CastRayVsNode(aRay, child, aOutHit, aHitAnything, aClosestSoFar);
	}


	return aHitAnything;
}

void Octtree::InsertHitableIntoNode(const Hitable* aHitable, Node* aNode)
{
	if (aNode->myDepth == 0)
	{
		aNode->myObjects.Add(aHitable);
		return;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (FirstAABBContainedBySecond(aHitable->myAABB, aNode->myChildAABBs[i]))
		{
			if (aNode->myChildNodes[i] == nullptr)
				aNode->myChildNodes[i] = new Node(aNode->myDepth - 1, aNode->myChildAABBs[i]);

			InsertHitableIntoNode(aHitable, aNode->myChildNodes[i]);
			return;
		}
	}

	aNode->myObjects.Add(aHitable);
}

//////////////////////////////////////////////////////////////////////////

void CollisionWorld::AddObject(const Sphere& aSphere)
{
	mySpheres.Add(aSphere);
}

void CollisionWorld::AddObject(const XY_Rect& aRect)
{
	myXYRects.Add(aRect);
}

bool CollisionWorld::CastRay(const Ray& aRay, RayHit& aOutHit) const
{
#if 1
	return myOctree.CastRay(aRay, aOutHit);
#else

	bool hitanything = false;
	float closestSoFar = FLT_MAX;

	for (const Sphere& sphere : mySpheres)
	{
		if(sphere.Hit(aRay, 0.001f, closestSoFar, aOutHit))
			hitanything = true;
	}

	for (const XY_Rect& rect : myXYRects)
	{
		if (rect.Hit(aRay, 0.01f, closestSoFar, aOutHit))
			hitanything = true;
	}

	return hitanything;
#endif
}

void CollisionWorld::BuildOctree()
{
	for (Sphere& sphere : mySpheres)
	{
		sphere.BuildAABB();
		myOctree.AddHitable(&sphere);
	}

	for (XY_Rect& rect : myXYRects)
	{
		rect.BuildAABB();
		myOctree.AddHitable(&rect);
	}
}

void CollisionWorld::ClearOctree()
{
	myOctree.Clear();
}

void CollisionWorld::ClearWorld()
{
	ClearOctree();
	mySpheres.RemoveAll();
	myXYRects.RemoveAll();
}