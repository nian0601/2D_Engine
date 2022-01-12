#pragma once

#include <cmath>
#include <assert.h>

template<typename T>
class FW_Vector3
{
public:
	FW_Vector3() { x = 0; y = 0; z = 0; }
	FW_Vector3(T aValue) { x = aValue; y = aValue; z = aValue; }
	FW_Vector3(T aX, T aY, T aZ) { x = aX; y = aY; z = aZ; }

	FW_Vector3<T> operator-()
	{
		FW_Vector3<T> tmp = *this;
		tmp.x = -x;
		tmp.y = -y;
		tmp.z = -z;
		return tmp;
	}

	T x;
	T y;
	T z;
};

template <typename T>
T Length(const FW_Vector3<T>& aVector)
{
	return T(sqrt(Length2(aVector)));
}

template <typename T>
T Length2(const FW_Vector3<T>& aVector)
{
	return Dot(aVector, aVector);
}

template <typename T>
void Normalize(FW_Vector3<T>& aVector)
{
	aVector = GetNormalized(aVector);
}

template <typename T>
FW_Vector3<T> GetNormalized(const FW_Vector3<T>& aVector)
{
	T length = Length(aVector);
	if (length == 0)
	{
		return FW_Vector3<T>();
	}
	return FW_Vector3<T>(aVector / length);
}

template <typename T>
T Dot(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	return aFirstVector.x * aSecondVector.x + aFirstVector.y * aSecondVector.y + aFirstVector.z * aSecondVector.z;
}

template <typename T>
FW_Vector3<T> Cross(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	return FW_Vector3<T>(
		aFirstVector.y * aSecondVector.z - aFirstVector.z * aSecondVector.y,
		aFirstVector.z * aSecondVector.x - aFirstVector.x * aSecondVector.z,
		aFirstVector.x * aSecondVector.y - aFirstVector.y * aSecondVector.x);
}

template <typename T>
FW_Vector3<T> ReflectAround(const FW_Vector3<T>& aVector, const FW_Vector3<T>& aReflectionVector)
{
	return aVector - static_cast<T>(2) * aReflectionVector * Dot(aReflectionVector, aVector);
}

// operator with vector
template <typename T>
bool operator==(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	return aFirstVector.x == aSecondVector.x && aFirstVector.y == aSecondVector.y && aFirstVector.z == aSecondVector.z;
}

template <typename T>
bool operator!=(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	return !(aFirstVector == aSecondVector);
}

template <typename T>
FW_Vector3<T> operator+(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	FW_Vector3<T> result(aFirstVector);
	result += aSecondVector;
	return result;
}
template <typename T>
FW_Vector3<T> operator+=(FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	aFirstVector.x += aSecondVector.x;
	aFirstVector.y += aSecondVector.y;
	aFirstVector.z += aSecondVector.z;
	return aFirstVector;
}

template <typename T>
FW_Vector3<T> operator-(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	FW_Vector3<T> result(aFirstVector);
	result -= aSecondVector;
	return result;
}
template <typename T>
FW_Vector3<T> operator-=(FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	aFirstVector.x -= aSecondVector.x;
	aFirstVector.y -= aSecondVector.y;
	aFirstVector.z -= aSecondVector.z;
	return aFirstVector;
}

template <typename T>
FW_Vector3<T> operator*(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	FW_Vector3<T> result(aFirstVector);
	result *= aSecondVector;
	return result;
}
template <typename T>
FW_Vector3<T> operator*=(FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	aFirstVector.x *= aSecondVector.x;
	aFirstVector.y *= aSecondVector.y;
	aFirstVector.z *= aSecondVector.z;
	return aFirstVector;
}

template <typename T>
FW_Vector3<T> operator/(const FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	FW_Vector3<T> result(aFirstVector);
	result /= aSecondVector;
	return result;
}
template <typename T>
FW_Vector3<T> operator/=(FW_Vector3<T>& aFirstVector, const FW_Vector3<T>& aSecondVector)
{
	assert(aSecondVector.x != 0 && aSecondVector.y != 0 && aSecondVector.z != 0 && "Division by zero.");
	aFirstVector.x /= aSecondVector.x;
	aFirstVector.y /= aSecondVector.y;
	aFirstVector.z /= aSecondVector.z;
	return aFirstVector;
}

// operator with scalar
template <typename T>
FW_Vector3<T> operator+(const FW_Vector3<T>& aVector, T aScalar)
{
	FW_Vector3<T> result(aVector);
	result += aScalar;
	return result;
}
template <typename T>
FW_Vector3<T> operator+(T aScalar, const FW_Vector3<T>& aVector)
{
	return aVector + aScalar;
}
template <typename T>
FW_Vector3<T> operator+=(FW_Vector3<T>& aVector, T aScalar)
{
	aVector.x += aScalar;
	aVector.y += aScalar;
	aVector.z += aScalar;
	return aVector;
}

template <typename T>
FW_Vector3<T> operator-(const FW_Vector3<T>& aVector, T aScalar)
{
	FW_Vector3<T> result(aVector);
	result -= aScalar;
	return result;
}

template <typename T>
FW_Vector3<T> operator-=(FW_Vector3<T>& aVector, T aScalar)
{
	aVector.x -= aScalar;
	aVector.y -= aScalar;
	aVector.z -= aScalar;
	return aVector;
}

template <typename T>
FW_Vector3<T> operator*(const FW_Vector3<T>& aVector, T aScalar)
{
	FW_Vector3<T> result(aVector);
	result *= aScalar;
	return result;
}
template <typename T>
FW_Vector3<T> operator*(T aScalar, const FW_Vector3<T>& aVector)
{
	return aVector * aScalar;
}
template <typename T>
FW_Vector3<T> operator*=(FW_Vector3<T>& aVector, T aScalar)
{
	aVector.x *= aScalar;
	aVector.y *= aScalar;
	aVector.z *= aScalar;
	return aVector;
}

template <typename T>
FW_Vector3<T> operator/(const FW_Vector3<T>& aVector, T aScalar)
{
	FW_Vector3<T> result(aVector);
	result /= aScalar;
	return result;
}
template <typename T>
FW_Vector3<T> operator/=(FW_Vector3<T>& aVector, T aScalar)
{
	assert(aScalar != 0 && "Division by zero.");
	aVector.x /= aScalar;
	aVector.y /= aScalar;
	aVector.z /= aScalar;
	return aVector;
}

template <typename T>
FW_Vector3<T> operator-(const FW_Vector3<T>& aVector)
{
	return FW_Vector3<T>(-aVector.x, -aVector.y, -aVector.z);
}

//typedefs
typedef FW_Vector3<float> Vector3f;
typedef FW_Vector3<int> Vector3i;


inline Vector3f Refract(const Vector3f& aUV, const Vector3f& aNormal, float aEtaiOverEtat)
{
	float cosTheta = fmin(Dot(-aUV, aNormal), 1.f);
	Vector3f rOutPerp = aEtaiOverEtat * (aUV + cosTheta * aNormal);
	Vector3f rOutParallel = -sqrt(fabs(1.f - Length2(rOutPerp))) * aNormal;
	return rOutPerp + rOutParallel;
}