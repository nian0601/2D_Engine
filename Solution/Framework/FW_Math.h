#pragma once
#include <stdlib.h>
#include "FW_Vector2.h"
#include "FW_Vector3.h"
#define FW_PI 3.14159265358979323846f

inline float FW_DegreesToRadians(float anAngle)
{
	return anAngle * FW_PI / 180.f;
}

inline float FW_RadiansToDegrees(float anAngle)
{
	return anAngle * 180.f / FW_PI;
}

inline float FW_RandFloat()
{
	return static_cast<float>(rand() % 10000) / 10000.f;
}

inline float FW_RandFloat(float aMin, float aMax) 
{
	return aMin + (aMax - aMin) * FW_RandFloat();
}

inline float FW_RandClamped()
{
	return FW_RandFloat() - FW_RandFloat();
}

inline int FW_RandInt(int aMin, int aMax)
{
	return (rand() & (aMax - aMin + 1)) + aMin;
}

inline Vector3f FW_RandomVector3f()
{
	return { FW_RandFloat(), FW_RandFloat(), FW_RandFloat() };
}

inline Vector3f FW_RandomVector3f(float aMin, float aMax)
{
	return { FW_RandFloat(aMin, aMax), FW_RandFloat(aMin, aMax), FW_RandFloat(aMin, aMax) };
}

inline Vector3f FW_RandomUnitSphereVector()
{
	while (true)
	{
		Vector3f v = FW_RandomVector3f(-1.f, 1.f);
		if(Length2(v) >= 1.f) continue;

		return v;
	}
}

inline Vector3f FW_RandomUnitVector()
{
	return GetNormalized(FW_RandomUnitSphereVector());
}

inline Vector3f FW_RandomInHemisphereVector(const Vector3f& aNormal)
{
	Vector3f inUnitSphere = FW_RandomUnitSphereVector();

	if (Dot(inUnitSphere, aNormal) > 0.f)
		return inUnitSphere;

	return -inUnitSphere;
}

inline Vector3f FW_RandomInUnitDisk()
{
	while (true)
	{
		Vector3f v = Vector3f(FW_RandFloat(-1.f, 1.f), FW_RandFloat(-1.f, 1.f), 0.f);
		if(Length2(v) >= 1.f) continue;

		return v;
	}
}

inline float FW_RoundToInt(float aFloat)
{
	int intCast = static_cast<int>(aFloat > 0.f ? aFloat + 0.5f : aFloat - 0.5f);
	return static_cast<float>(intCast);
}

inline void FW_RoundToInt(Vector2f& aVector)
{
	aVector.x = FW_RoundToInt(aVector.x);
	aVector.y = FW_RoundToInt(aVector.y);
}

template <typename T>
inline T FW_Min(const T& aFirst, const T& aSecond)
{
	return aFirst < aSecond ? aFirst : aSecond;
}

template <typename T>
inline T FW_Max(const T& aFirst, const T& aSecond)
{
	return aFirst > aSecond ? aFirst : aSecond;
}

inline float FW_Lerp(float aStart, float aEnd, float aAlpha)
{
	return (aStart + (aEnd - aStart) * aAlpha);
}

template <typename T>
inline T FW_Clamp(const T& aValue, const T& aMin, const T& aMax)
{
	if (aValue < aMin) return aMin;
	if (aValue > aMax) return aMax;
	return aValue;
}