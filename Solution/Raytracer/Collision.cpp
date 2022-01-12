#include "Collision.h"
#include <float.h>

bool AABBvsAABB(const AABB& aFirst, const AABB& aSecond)
{
	if (aFirst.myMaxPos.x < aSecond.myMinPos.x) return false;
	if (aFirst.myMaxPos.y < aSecond.myMinPos.y) return false;
	if (aFirst.myMaxPos.z < aSecond.myMinPos.z) return false;
	if (aFirst.myMinPos.x > aSecond.myMaxPos.x) return false;
	if (aFirst.myMinPos.y > aSecond.myMaxPos.y) return false;
	if (aFirst.myMinPos.z > aSecond.myMaxPos.z) return false;

	return true;
}

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

bool RayVsSphere(const Ray& aRay, const Sphere& aSphere, float aMinT, float aMaxT, RayHit& aHitRecord)
{
	Vector3f oc = aRay.myPosition - aSphere.myPosition;
	float a = Length2(aRay.myDirection);
	float halfB = Dot(oc, aRay.myDirection);
	float c = Length2(oc) - aSphere.myRadius * aSphere.myRadius;
	float discriminant = halfB * halfB - a * c;

	if (discriminant < 0)
		return false;

	float squareDiscriminant = sqrt(discriminant);
	float root = (-halfB - squareDiscriminant) / a;
	if (root < aMinT || root > aMaxT)
	{
		root = (-halfB + squareDiscriminant) / a;
		if (root < aMinT || root > aMaxT)
			return false;
	}

	aHitRecord.myT = root;
	aHitRecord.myPosition = aRay.PositionAt(aHitRecord.myT);
	Vector3f outwardNormal = (aHitRecord.myPosition - aSphere.myPosition) / aSphere.myRadius;
	aHitRecord.SetFaceNormal(aRay, outwardNormal);
	aHitRecord.myMaterial = aSphere.myMaterial;
	return true;
}

bool SphereContainedByAABB(const Sphere& aSphere, const AABB& anAABB)
{
	const Vector3f sphereMin = aSphere.myPosition - aSphere.myRadius;
	const Vector3f sphereMax = aSphere.myPosition + aSphere.myRadius;

	if (sphereMin.x < anAABB.myMinPos.x) return false;
	if (sphereMin.y < anAABB.myMinPos.y) return false;
	if (sphereMin.z < anAABB.myMinPos.z) return false;

	if (sphereMax.x > anAABB.myMaxPos.x) return false;
	if (sphereMax.y > anAABB.myMaxPos.y) return false;
	if (sphereMax.z > anAABB.myMaxPos.z) return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////

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

	//for (int i = 0; i < 8; ++i)
	//	myChildNodes.Add(new Node(aDepth - 1, childAABBs[i]));
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

void Octtree::AddSphere(const Sphere* aSphere)
{
	InsertSphereIntoNode(aSphere, myRootNode);
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
	for (const Sphere* sphere : aNode->myObjects)
	{
		if (RayVsSphere(aRay, *sphere, 0.01f, aClosestSoFar, aOutHit))
		{
			aHitAnything = true;
			aClosestSoFar = aOutHit.myT;
		}
	}

	for (Node* child : aNode->myChildNodes)
	{
		if(child && RayHitAABB(aRay, child->myAABB))
			CastRayVsNode(aRay, child, aOutHit, aHitAnything, aClosestSoFar);
	}


	return aHitAnything;
}

void Octtree::InsertSphereIntoNode(const Sphere* aSphere, Node* aNode)
{
	if (aNode->myDepth == 0)
	{
		aNode->myObjects.Add(aSphere);
		return;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (SphereContainedByAABB(*aSphere, aNode->myChildAABBs[i]))
		{
			if (aNode->myChildNodes[i] == nullptr)
				aNode->myChildNodes[i] = new Node(aNode->myDepth - 1, aNode->myChildAABBs[i]);

			InsertSphereIntoNode(aSphere, aNode->myChildNodes[i]);
			return;
		}
	}

	aNode->myObjects.Add(aSphere);
}

//////////////////////////////////////////////////////////////////////////

void CollisionWorld::AddSphere(const Sphere& aSphere)
{
	mySpheres.Add(aSphere);
}

bool CollisionWorld::CastRay(const Ray& aRay, RayHit& aOutHit) const
{
#if 1
	return myOctree.CastRay(aRay, aOutHit);
#else
	bool hitAnything = false;
	float closestSoFar = FLT_MAX;
	
	for (const Sphere& sphere : mySpheres)
	{
		if (RayVsSphere(aRay, sphere, 0.01f, closestSoFar, aOutHit))
		{
			hitAnything = true;
			closestSoFar = aOutHit.myT;
		}
	}
	
	return hitAnything;
#endif
}

void CollisionWorld::BuildOctree()
{
	for (const Sphere& sphere : mySpheres)
		myOctree.AddSphere(&sphere);
}

void CollisionWorld::ClearOctree()
{
	myOctree.Clear();
}

void CollisionWorld::ClearWorld()
{
	ClearOctree();
	mySpheres.RemoveAll();
}
