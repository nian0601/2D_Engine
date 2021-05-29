#pragma once

#include <cmath>
#include <assert.h>

template<typename T>
class FW_Vector2
{
public:
	FW_Vector2() { x = 0; y = 0; }
	FW_Vector2(T aValue) { x = aValue; y = aValue; }
	FW_Vector2(T aX, T aY) { x = aX; y = aY; }
	~FW_Vector2() {};

	FW_Vector2<T> operator-()
	{
		FW_Vector2<T> tmp = *this;
		tmp.x = -x;
		tmp.y = -y;
		return tmp;
	}

	T x;
	T y;
};

template <typename T>
T Length(const FW_Vector2<T>& aVector)
{
	return T(sqrt(Length2(aVector)));
}

template <typename T>
T Length2(const FW_Vector2<T>& aVector)
{
	return Dot(aVector, aVector);
}

template <typename T>
void Normalize(FW_Vector2<T>& aVector)
{
	aVector = GetNormalized(aVector);
}

template <typename T>
FW_Vector2<T> GetNormalized(const FW_Vector2<T>& aVector)
{
	T length = Length(aVector);
	if (length == 0)
	{
		return FW_Vector2<T>();
	}
	return FW_Vector2<T>(aVector / length);
}

template <typename T>
T Dot(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return aFirstVector.x * aSecondVector.x + aFirstVector.y * aSecondVector.y;
}

template <typename T>
FW_Vector2<T> ReflectAround(const FW_Vector2<T>& aVector, const FW_Vector2<T>& aReflectionVector)
{
	return aVector - static_cast<T>(2) * aReflectionVector * Dot(aReflectionVector, aVector);
}

// operator with vector
template <typename T>
bool operator==(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return aFirstVector.x == aSecondVector.x && aFirstVector.y == aSecondVector.y;
}

template <typename T>
bool operator!=(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	return !(aFirstVector == aSecondVector);
}

template <typename T>
FW_Vector2<T> operator+(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result += aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator+=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	aFirstVector.x += aSecondVector.x;
	aFirstVector.y += aSecondVector.y;
	return aFirstVector;
}

template <typename T>
FW_Vector2<T> operator-(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result -= aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator-=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	aFirstVector.x -= aSecondVector.x;
	aFirstVector.y -= aSecondVector.y;
	return aFirstVector;
}

template <typename T>
FW_Vector2<T> operator*(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result *= aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator*=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	aFirstVector.x *= aSecondVector.x;
	aFirstVector.y *= aSecondVector.y;
	return aFirstVector;
}

template <typename T>
FW_Vector2<T> operator/(const FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	FW_Vector2<T> result(aFirstVector);
	result /= aSecondVector;
	return result;
}
template <typename T>
FW_Vector2<T> operator/=(FW_Vector2<T>& aFirstVector, const FW_Vector2<T>& aSecondVector)
{
	assert(aSecondVector.x != 0 && aSecondVector.y != 0 && "Division by zero.");
	aFirstVector.x /= aSecondVector.x;
	aFirstVector.y /= aSecondVector.y;
	return aFirstVector;
}

// operator with scalar
template <typename T>
FW_Vector2<T> operator+(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result += aScalar;
	return result;
}
template <typename T>
FW_Vector2<T> operator+(T aScalar, const FW_Vector2<T>& aVector)
{
	return aVector + aScalar;
}
template <typename T>
FW_Vector2<T> operator+=(FW_Vector2<T>& aVector, T aScalar)
{
	aVector.x += aScalar;
	aVector.y += aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator-(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result -= aScalar;
	return result;
}

template <typename T>
FW_Vector2<T> operator-=(FW_Vector2<T>& aVector, T aScalar)
{
	aVector.x -= aScalar;
	aVector.y -= aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator*(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result *= aScalar;
	return result;
}
template <typename T>
FW_Vector2<T> operator*(T aScalar, const FW_Vector2<T>& aVector)
{
	return aVector * aScalar;
}
template <typename T>
FW_Vector2<T> operator*=(FW_Vector2<T>& aVector, T aScalar)
{
	aVector.x *= aScalar;
	aVector.y *= aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator/(const FW_Vector2<T>& aVector, T aScalar)
{
	FW_Vector2<T> result(aVector);
	result /= aScalar;
	return result;
}
template <typename T>
FW_Vector2<T> operator/=(FW_Vector2<T>& aVector, T aScalar)
{
	assert(aScalar != 0 && "Division by zero.");
	aVector.x /= aScalar;
	aVector.y /= aScalar;
	return aVector;
}

template <typename T>
FW_Vector2<T> operator-(const FW_Vector2<T>& aVector)
{
	return FW_Vector2<T>(-aVector.x, -aVector.y);
}

//typedefs
typedef FW_Vector2<float> Vector2f;
typedef FW_Vector2<int> Vector2i;