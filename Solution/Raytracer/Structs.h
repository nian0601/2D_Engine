#pragma once
#include "FW_Vector3.h"
#include <FW_Math.h>
#include <FW_String.h>

struct Material
{
	enum MaterialType
	{
		Lambertian,
		Metal,
		Dialectric,
		LightSource,
	};

	Vector3f myColor = { 1.f, 1.f, 1.f };
	int myMaterialType = Lambertian;
	float myMaterialParameter = 1.f;

	static Material MakeLambertian(const Vector3f& aColor)
	{
		Material result;
		result.myMaterialType = Lambertian;
		result.myColor = aColor;
		return result;
	}

	static Material MakeMetal(const Vector3f& aColor, float aFuzz)
	{
		Material result;
		result.myMaterialType = Metal;
		result.myColor = aColor;
		result.myMaterialParameter = aFuzz;
		return result;
	}

	static Material MakeDialectric(float aFuzz)
	{
		Material result;
		result.myMaterialType = Dialectric;
		result.myMaterialParameter = aFuzz;
		return result;
	}

	static Material MakeLight(const Vector3f& aColor)
	{
		Material result;
		result.myMaterialType = LightSource;
		result.myColor = aColor;
		return result;
	}
};

struct Ray
{
	Vector3f myPosition;
	Vector3f myDirection;

	Vector3f PositionAt(float t) const
	{
		return myPosition + myDirection * t;
	}
};

struct RayHit
{
	Vector3f myPosition;
	Vector3f myNormal;
	float myT;
	bool myIsFrontFace;
	Material myMaterial;

	void SetFaceNormal(const Ray& aRay, const Vector3f& aOutwardNormal)
	{
		myIsFrontFace = Dot(aRay.myDirection, aOutwardNormal) < 0.f;
		myNormal = myIsFrontFace ? aOutwardNormal : -aOutwardNormal;
	}
};

struct AABB
{
	Vector3f myCenterPos;
	Vector3f myExtents;
	Vector3f myMinPos;
	Vector3f myMaxPos;
};

inline AABB AABBFromMinCornerAndSize(const Vector3f& aMinCorner, const Vector3f& aSize)
{
	AABB result;
	result.myExtents = aSize;
	result.myMinPos = aMinCorner;
	result.myMaxPos = aMinCorner + aSize;
	result.myCenterPos = aMinCorner + aSize * 0.f;
	return result;
}

inline AABB AABBFromExtents(const Vector3f& aPosition, const Vector3f& someExtents)
{
	AABB result;
	result.myCenterPos = aPosition;
	result.myExtents = someExtents;
	result.myMinPos = result.myCenterPos - result.myExtents * 0.5f;
	result.myMaxPos = result.myCenterPos + result.myExtents * 0.5f;
	return result;
}

inline AABB AABBFromPoints(const Vector3f& aMin, const Vector3f& aMax)
{
	AABB result;
	result.myMinPos = aMin;
	result.myMaxPos = aMax;
	result.myExtents = result.myMaxPos - result.myMinPos;
	result.myCenterPos = result.myMinPos + result.myExtents;
	return result;
}

struct Camera
{
	Vector3f myPosition;
	Vector3f myHorizontal;
	Vector3f myVertical;
	Vector3f myLowerLeftCorner;
	Vector3f myU;
	Vector3f myV;
	Vector3f myW;
	float myLensRadius;

	void Setup(
		const Vector3f& aLookFrom,
		const Vector3f& aLookAt,
		const Vector3f& aUp,
		float aVertialFov,
		float anAspectRatio,
		float aAperature,
		float aFocusDistance)
	{
		float theta = FW_DegreesToRadians(aVertialFov);
		float h = tan(theta / 2.f);
		float viewportHeight = 2.f * h;
		float viewportWidth = viewportHeight * anAspectRatio;

		myW = GetNormalized(aLookFrom - aLookAt);
		myU = GetNormalized(Cross(aUp, myW));
		myV = Cross(myW, myU);

		myPosition = aLookFrom;
		myHorizontal = aFocusDistance * viewportWidth * myU;
		myVertical = aFocusDistance * viewportHeight * myV;
		myVertical.y *= -1.f;
		myLowerLeftCorner = myPosition - myHorizontal * 0.5f - myVertical * 0.5f - aFocusDistance * myW;

		myLensRadius = aAperature * 0.5f;
	}

	Ray GetRay(float s, float t)
	{
		Vector3f rd = myLensRadius * FW_RandomInUnitDisk();
		Vector3f offset = myU * rd.x + myV * rd.y;

		return { myPosition + offset, myLowerLeftCorner + s * myHorizontal + t * myVertical - myPosition - offset };
	}
};
