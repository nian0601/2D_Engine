#pragma once

#include "FW_Vector2.h"

struct FW_Matrix22
{
	union
	{
		struct
		{
			float my00, my01;
			float my10, my11;
		};

		float m[2][2];
		float v[4];
	};

	FW_Matrix22() 
		: my00(0.f), my01(0.f)
		, my10(0.f), my11(0.f)
	{}

	FW_Matrix22(float a00, float a01, float a10, float a11)
		: my00(a00), my01(a01)
		, my10(a10), my11(a11)
	{}

	FW_Matrix22(float aRadians)
	{
		Set(aRadians);
	}
	
	~FW_Matrix22()
	{}

	void Set(float aRadians)
	{
		float c = cos(aRadians);
		float s = sin(aRadians);

		my00 = c;
		my01 = -s;

		my10 = s;
		my11 = c;
	}

	FW_Matrix22 Transpose() const
	{
		return FW_Matrix22(my00, my10, my01, my11);
	}

	const Vector2f operator*(const Vector2f& aRhs) const
	{
		return Vector2f(my00 * aRhs.x + my01 * aRhs.y, my10 * aRhs.x + my11 * aRhs.y);
	}

	const FW_Matrix22 operator*(const FW_Matrix22& aRhs) const
	{
		// [00 01]  [00 01]
		// [10 11]  [10 11]

		return FW_Matrix22(
			m[0][0] * aRhs.m[0][0] + m[0][1] * aRhs.m[1][0],
			m[0][0] * aRhs.m[0][1] + m[0][1] * aRhs.m[1][1],
			m[1][0] * aRhs.m[0][0] + m[1][1] * aRhs.m[1][0],
			m[1][0] * aRhs.m[0][1] + m[1][1] * aRhs.m[1][1]
		);
	}
};
