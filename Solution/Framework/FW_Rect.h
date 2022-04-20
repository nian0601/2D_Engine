#pragma once
#include "FW_Vector2.h"

template <typename T>
struct FW_Rect
{
	FW_Vector2<T> myCenterPos;
	FW_Vector2<T> myExtents;
	FW_Vector2<T> myTopLeft;
	FW_Vector2<T> myBottomRight;
};

template <typename T>
FW_Rect<T> MakeRect(T aX1, T aY1, T aX2, T aY2)
{
	FW_Rect<T> rect;
	rect.myTopLeft = { aX1, aY1 };
	rect.myBottomRight = { aX2, aY2 };
	rect.myExtents = { aX2 - aX1, aY2 - aY1 };
	rect.myCenterPos = rect.myTopLeft + rect.myExtents / static_cast<T>(2.f);
	return rect;
}

template <typename T>
FW_Rect<T> MakeRect(const FW_Vector2<T>& aCenter, const FW_Vector2<T>& someExtents)
{
	FW_Rect<T> rect;
	rect.myExtents = someExtents;
	SetRectPosition(rect, aCenter);
	return rect;
}

template <typename T>
FW_Rect<T> MakeRectFromPoints(const FW_Vector2<T>& aPoint1, const FW_Vector2<T>& aPoint2)
{
	T minX;
	T maxX;
	T minY;
	T maxY;

	if (aPoint1.x < aPoint2.x)
	{
		minX = aPoint1.x;
		maxX = aPoint2.x;
	}
	else
	{ 
		minX = aPoint2.x;
		maxX = aPoint1.x;
	}

	if (aPoint1.y < aPoint2.y)
	{
		minY = aPoint1.y;
		maxY = aPoint2.y;
	}
	else
	{
		minY = aPoint2.y;
		maxY = aPoint1.y;
	}

	return MakeRect(minX, minY, maxX, maxY);
}

template <typename T>
void MoveRect(FW_Rect<T>& aRect, const FW_Vector2<T>& aMoveAmount)
{
	aRect.myCenterPos += aMoveAmount;
	aRect.myTopLeft = aRect.myCenterPos - aRect.myExtents / static_cast<T>(2.f);
	aRect.myBottomRight = aRect.myCenterPos + aRect.myExtents / static_cast<T>(2.f);
}

template <typename T>
void SetRectPosition(FW_Rect<T>& aRect, const FW_Vector2<T>& aNewPosition)
{
	aRect.myCenterPos = aNewPosition;
	aRect.myTopLeft = aRect.myCenterPos - aRect.myExtents / static_cast<T>(2.f);
	aRect.myBottomRight = aRect.myCenterPos + aRect.myExtents / static_cast<T>(2.f);
}

template <typename T>
bool Contains(const FW_Rect<T>& aRect, const FW_Vector2<T>& aPosition)
{
	if (aPosition.x < aRect.myTopLeft.x) return false;
	if (aPosition.x > aRect.myBottomRight.x) return false;

	if (aPosition.y < aRect.myTopLeft.y) return false;
	if (aPosition.y > aRect.myBottomRight.y) return false;

	return true;
}

template <typename T>
bool Collides(const FW_Rect<T>& a, const FW_Rect<T>& b)
{
	if (a.myBottomRight.x < b.myTopLeft.x) return false;
	if (a.myTopLeft.x > b.myBottomRight.x) return false;

	if (a.myTopLeft.y > b.myBottomRight.y) return false;
	if (a.myBottomRight.y < b.myTopLeft.y) return false;

	return true;
}

//typedefs
typedef FW_Rect<float> Rectf;
typedef FW_Rect<int> Recti;