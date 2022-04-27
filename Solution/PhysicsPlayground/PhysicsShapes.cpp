#include "PhysicsShapes.h"
#include "PhysicsWorld.h"
#include <FW_Math.h>
#include <FW_Renderer.h>
#include <float.h>
#include <FW_Assert.h>
#include <FW_String.h>

namespace Collision_Private
{
	bool CircleVsPolygon(const CircleShape& aCircle, const PolygonShape& aPolygon, Manifold& aManifold)
	{
		const CircleShape& A = aCircle;
		const PolygonShape& B = aPolygon;
		aManifold.myContactCount = 0;

		// Transform center of the circle into B's space
		Vector2f center = A.myObject->myPosition;
		center = B.myModelSpace.Transpose() * (center - B.myObject->myPosition);

		// Find the edge with minimum penetration
		float separation = -FLT_MAX;
		int faceNormal = 0;
		for (int i = 0; i < B.myVertexCount; ++i)
		{
			float s = Dot(B.myNormals[i], center - B.myVertices[i]);
			if (s > A.myRadius)
				return false;

			if (s > separation)
			{
				separation = s;
				faceNormal = i;
			}
		}

		Vector2f v1 = B.myVertices[faceNormal];
		int i2 = faceNormal + 1 >= B.myVertexCount ? 0 : faceNormal + 1;
		Vector2f v2 = B.myVertices[i2];

		if (separation < FW_EPSILON)
		{
			aManifold.myObjectA = A.myObject;
			aManifold.myObjectB = B.myObject;

			aManifold.myContactCount = 1;
			aManifold.myHitNormal = -(B.myModelSpace * B.myNormals[faceNormal]);
			aManifold.myContacts[0] = aManifold.myHitNormal * A.myRadius + A.myObject->myPosition;
			aManifold.myPenetrationDepth = A.myRadius;
			return true;
		}

		float dot1 = Dot(center - v1, v2 - v1);
		float dot2 = Dot(center - v2, v1 - v2);
		aManifold.myPenetrationDepth = A.myRadius - separation;

		if (dot1 <= 0.f)
		{
			if (Length2(center - v1) > FW_Square(A.myRadius))
				return false;

			aManifold.myContactCount = 1;

			aManifold.myHitNormal = v1 - center;
			aManifold.myHitNormal = B.myModelSpace * aManifold.myHitNormal;
			Normalize(aManifold.myHitNormal);

			v1 = B.myModelSpace * v1 + B.myObject->myPosition;
			aManifold.myContacts[0] = v1;
		}
		else if (dot2 <= 0.f)
		{
			if (Length2(center - v2) > FW_Square(A.myRadius))
				return false;

			aManifold.myContactCount = 1;

			aManifold.myHitNormal = v2 - center;
			aManifold.myHitNormal = B.myModelSpace * aManifold.myHitNormal;
			Normalize(aManifold.myHitNormal);

			v2 = B.myModelSpace * v2 + B.myObject->myPosition;
			aManifold.myContacts[0] = v2;
		}
		else
		{
			Vector2f n = B.myNormals[faceNormal];
			if (Dot(center - v1, n) > A.myRadius)
				return false;

			aManifold.myContactCount = 1;

			n = B.myModelSpace * n;
			aManifold.myHitNormal = -n;
			aManifold.myContacts[0] = aManifold.myHitNormal * A.myRadius + A.myObject->myPosition;
		}

		aManifold.myObjectA = A.myObject;
		aManifold.myObjectB = B.myObject;

		return true;
	}
}

bool CircleShape::RunCollision(const Shape& aShape, Manifold& aManifold) const
{
	return aShape.TestCollision(*this, aManifold);
}

bool CircleShape::TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const
{
	Vector2f collisionVector = aCircleShape.myObject->myPosition - myObject->myPosition;
	const float distance = Length(collisionVector);
	if (distance >= myRadius + aCircleShape.myRadius)
	{
		aManifold.myContactCount = 0;
		return false;
	}

	aManifold.myObjectA = myObject;
	aManifold.myObjectB = aCircleShape.myObject;
	aManifold.myContactCount = 1;

	if (distance == 0.f)
	{
		aManifold.myPenetrationDepth = myRadius;
		aManifold.myHitNormal = { 1.f, 0.f };
		aManifold.myContacts[0] = myObject->myPosition;
	}
	else
	{
		aManifold.myPenetrationDepth = distance - (myRadius + aCircleShape.myRadius);
		aManifold.myHitNormal = GetNormalized(collisionVector);
		aManifold.myContacts[0] = aManifold.myHitNormal * myRadius + myObject->myPosition;
	}

	return true;
}

bool CircleShape::TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const
{
	return Collision_Private::CircleVsPolygon(*this, aPolygonShape, aManifold);
}

void CircleShape::Render() const
{
	FW_Renderer::RenderCircle(myObject->myPosition, myRadius, myObject->myColor);

	FW_Matrix22 orientation(myObject->myOrientation);

	Vector2f endPoint(0.f, 1.f);
	endPoint = orientation * endPoint;
	endPoint *= myRadius;
	endPoint += myObject->myPosition;
	FW_Renderer::RenderLine(myObject->myPosition, endPoint, 0xFF000000);
}

void CircleShape::ComputeMass(float aDensity)
{
	myObject->SetMass(FW_PI * myRadius * myRadius * aDensity);
	myObject->SetInertia(myObject->myMass * myRadius * myRadius);
}

//////////////////////////////////////////////////////////////////////////
//
// Polygon-collision reference
// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032
// https://github.com/tutsplus/ImpulseEngine/blob/master/Collision.cpp
//

float FindAxisLeastPenetration(const PolygonShape& A, const PolygonShape& B, int& aFaceIndexOut)
{
	float bestDistance = -FLT_MAX;
	int bestIndex;

	for (int i = 0; i < A.myVertexCount; ++i)
	{
		// Get face-normal from A and transform to worldspace
		Vector2f n = A.myNormals[i];
		Vector2f nw = A.myModelSpace * n;

		// Transform normal into B's space
		FW_Matrix22 buT = B.myModelSpace.Transpose();
		n = buT * nw;

		Vector2f s = B.GetSupport(-n);

		// Get Vertex from A, transform to Worldspace and then to B's space
		Vector2f v = A.myVertices[i];
		v = A.myModelSpace * v + A.myObject->myPosition;
		v -= B.myObject->myPosition;
		v = buT * v;

		// Compute penetration-distance (in B's space)
		float d = Dot(n, s - v);
		if (d > bestDistance)
		{
			bestDistance = d;
			bestIndex = i;
		}
	}

	aFaceIndexOut = bestIndex;
	return bestDistance;
}

void FindIncidentFace(const PolygonShape& aRefPoly, const PolygonShape& aIncPoly, int aReferenceIndex, Vector2f* outFace)
{
	Vector2f referenceNormal = aRefPoly.myNormals[aReferenceIndex];

	// Transform normal into Incident's space
	referenceNormal = aRefPoly.myModelSpace * referenceNormal;
	referenceNormal = aIncPoly.myModelSpace.Transpose() * referenceNormal;

	// Find most anti-normal face on incident poly
	int incidentFace = 0;
	float minDot = FLT_MAX;
	for (int i = 0; i < aIncPoly.myVertexCount; ++i)
	{
		float dot = Dot(referenceNormal, aIncPoly.myNormals[i]);
		if (dot < minDot)
		{
			minDot = dot;
			incidentFace = i;
		}
	}

	outFace[0] = aIncPoly.myModelSpace * aIncPoly.myVertices[incidentFace] + aIncPoly.myObject->myPosition;
	incidentFace = incidentFace + 1 >= aIncPoly.myVertexCount ? 0 : incidentFace + 1;
	outFace[1] = aIncPoly.myModelSpace * aIncPoly.myVertices[incidentFace] + aIncPoly.myObject->myPosition;
}

int Clip(const Vector2f& aNormal, float c, Vector2f* aFace)
{
	int sp = 0;
	Vector2f out[2] = {
		aFace[0],
		aFace[1]
	};

	// Retrieve distance from each endpoint to the line
	// d = ax + by + c
	float d1 = Dot(aNormal, aFace[0]) - c;
	float d2 = Dot(aNormal, aFace[1]) - c;

	// If negative (behind plane), then clip
	if (d1 <= 0.f) out[sp++] = aFace[0];
	if (d2 <= 0.f) out[sp++] = aFace[1];

	if (d1 * d2 < 0.f)
	{
		float alpha = d1 / (d1 - d2);
		out[sp] = aFace[0] + alpha * (aFace[1] - aFace[0]);
		++sp;
	}

	aFace[0] = out[0];
	aFace[1] = out[1];

	FW_ASSERT(sp != 3);
	return sp;
}

bool BiasGreaterThan(float a, float b)
{
	const float biasRelative = 0.95f;
	const float biasAbsolute = 0.01f;
	return a >= b * biasRelative + a * biasAbsolute;
}

bool PolygonShape::RunCollision(const Shape& aShape, Manifold& aManifold) const
{
	return aShape.TestCollision(*this, aManifold);
}

bool PolygonShape::TestCollision(const CircleShape& aCircleShape, Manifold& aManifold) const
{
	bool result = Collision_Private::CircleVsPolygon(aCircleShape, *this, aManifold);
	return result;
}

bool PolygonShape::TestCollision(const PolygonShape& aPolygonShape, Manifold& aManifold) const
{
	const PolygonShape& A = *this;
	const PolygonShape& B = aPolygonShape;

	aManifold.myContactCount = 0;

	int faceA;
	float penetrationA = FindAxisLeastPenetration(A, B, faceA);
	if (penetrationA >= 0.f)
		return false;

	int faceB;
	float penetrationB = FindAxisLeastPenetration(B, A, faceB);
	if (penetrationB >= 0.f)
		return false;

	const PolygonShape* refPoly = &A;
	const PolygonShape* incPoly = &B;
	int referenceIndex = faceA;
	bool flip = false;

	if (BiasGreaterThan(penetrationA, penetrationB) == false)
	{
		refPoly = &B;
		incPoly = &A;
		referenceIndex = faceB;
		flip = true;
	}

	Vector2f incidentFace[2];
	FindIncidentFace(*refPoly, *incPoly, referenceIndex, incidentFace);

	Vector2f v1 = refPoly->myVertices[referenceIndex];
	referenceIndex = referenceIndex + 1 == refPoly->myVertexCount ? 0 : referenceIndex + 1;
	Vector2f v2 = refPoly->myVertices[referenceIndex];

	v1 = refPoly->myModelSpace * v1 + refPoly->myObject->myPosition;
	v2 = refPoly->myModelSpace * v2 + refPoly->myObject->myPosition;

	Vector2f sidePlaneNormal = GetNormalized(v2 - v1);

	Vector2f refFaceNormal = { sidePlaneNormal.y, -sidePlaneNormal.x };

	float refC = Dot(refFaceNormal, v1);
	float negSide = -Dot(sidePlaneNormal, v1);
	float posSide =  Dot(sidePlaneNormal, v2);

	if (Clip(-sidePlaneNormal, negSide, incidentFace) < 2)
		return false;

	if (Clip(sidePlaneNormal, posSide, incidentFace) < 2)
		return false;

	aManifold.myHitNormal = flip ? -refFaceNormal : refFaceNormal;
	aManifold.myPenetrationDepth = 0.f;

	int contactPoints = 0;

	float separation = Dot(refFaceNormal, incidentFace[0]) - refC;
	if (separation <= 0.f)
	{
		aManifold.myContacts[contactPoints] = incidentFace[0];
		aManifold.myPenetrationDepth += -separation;
		++contactPoints;
	}

	separation = Dot(refFaceNormal, incidentFace[1]) - refC;
	if (separation <= 0.f)
	{
		aManifold.myContacts[contactPoints] = incidentFace[1];
		aManifold.myPenetrationDepth += -separation;
		++contactPoints;
	}

	aManifold.myPenetrationDepth /= static_cast<float>(contactPoints);
	aManifold.myContactCount = contactPoints;
	aManifold.myObjectA = A.myObject;
	aManifold.myObjectB = B.myObject;
	return true;
}

void PolygonShape::Render() const
{
	FW_Matrix22 space;
	space.Set(myObject->myPreviousOrientation);

	Vector2f p1 = myObject->myPreviousPosition + space * myVertices[0];
	for (int i = 1; i < myVertexCount; ++i)
	{
		Vector2f p2 = myObject->myPreviousPosition + space * myVertices[i];
		FW_Renderer::RenderLine(p1, p2, myObject->myColor);
		p1 = p2;
	}

	Vector2f p2 = myObject->myPreviousPosition + space * myVertices[0];
	FW_Renderer::RenderLine(p1, p2, myObject->myColor);

	/*Vector2f p1 = myObject->myPosition + myModelSpace * myVertices[0];
	for (int i = 1; i < myVertexCount; ++i)
	{
		Vector2f p2 = myObject->myPosition + myModelSpace * myVertices[i];
		FW_Renderer::RenderLine(p1, p2, myObject->myColor);
		p1 = p2;
	}

	Vector2f p2 = myObject->myPosition + myModelSpace * myVertices[0];
	FW_Renderer::RenderLine(p1, p2, myObject->myColor);*/
}

void PolygonShape::SetOrientation(float aRadians)
{
	myModelSpace.Set(aRadians);
}

void PolygonShape::ComputeMass(float aDensity)
{
	Vector2f c;
	float area = 0.f;
	float inertia = 0.f;
	const float inv3 = 1.f / 3.f;

	for (int i = 0; i < myVertexCount; ++i)
	{
		Vector2f p1 = myVertices[i];
		int i2 = i + 1 < myVertexCount ? i + 1 : 0;
		Vector2f p2 = myVertices[i2];

		float D = Cross(p1, p2);
		float triangleArea = 0.5f * D;

		area += triangleArea;

		c += triangleArea * inv3 * (p1 + p2);

		float intx2 = p1.x * p1.x + p2.x * p1.x + p2.x * p2.x;
		float inty2 = p1.y * p1.y + p2.y * p1.y + p2.y * p2.y;
		inertia += (0.25f * inv3 * D) + (intx2 + inty2);
	}

	c *= 1.f / area;

	for (int i = 0; i < myVertexCount; ++i)
		myVertices[i] -= c;

	myObject->SetMass(aDensity * area);
	myObject->SetInertia(aDensity * inertia);
}

Vector2f PolygonShape::GetSupport(const Vector2f& aDirection) const
{
	float bestProjection = -FLT_MAX;
	Vector2f bestVertex;

	for (int i = 0; i < myVertexCount; ++i)
	{
		const Vector2f& v = myVertices[i];
		float projection = Dot(v, aDirection);

		if (projection > bestProjection)
		{
			bestVertex = v;
			bestProjection = projection;
		}
	}

	return bestVertex;
}

AABBShape::AABBShape(const Vector2f& aSize)
{
	Vector2f halfSize = aSize * 0.5f;

	myVertexCount = 4;
	myVertices[0] = { -halfSize.x, -halfSize.y };
	myVertices[1] = { halfSize.x, -halfSize.y };
	myVertices[2] = { halfSize.x,  halfSize.y };
	myVertices[3] = { -halfSize.x,  halfSize.y };
	myNormals[0] = { 0.f, -1.f };
	myNormals[1] = { 1.f,  0.f };
	myNormals[2] = { 0.f,  1.f };
	myNormals[3] = { -1.f,  0.f };

	myRect = MakeRect({ 0.f, 0.f }, aSize);
}

void AABBShape::Render() const
{
	myRect.myCenterPos = myObject->myPreviousPosition;
	FW_Renderer::RenderRect(myRect, myObject->myColor, myObject->myPreviousOrientation);
}
