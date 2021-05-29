#pragma once
#include <stdlib.h>
#include "FW_Vector2.h"
#define FW_PI 3.14159265358979323846f

inline float FW_RandFloat()
{
	return static_cast<float>(rand() % 10000) / 10000.f;
}

inline float FW_RandClamped()
{
	return FW_RandFloat() - FW_RandFloat();
}

inline int FW_RandInt(int aMin, int aMax)
{
	return (rand() & (aMax - aMin + 1)) + aMin;
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