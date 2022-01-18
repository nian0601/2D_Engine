#pragma once
#include "Structs.h"

namespace Materials
{
	bool ScatterRay(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay);
	Vector3f EmitFromHit(const RayHit& aHitRecord);
}
