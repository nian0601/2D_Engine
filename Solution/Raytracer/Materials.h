#pragma once
#include "Structs.h"

namespace Materials
{
	bool ScatterRay(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay);
	Vector3f EmitFromHit(const RayHit& aHitRecord);

	//bool ScatterLambertian(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay);
	//bool ScatterMetal(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay);
	//bool ScatterDialectric(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay);
	//bool ScatterLightSource(const Ray& aRayIn, const RayHit& aHitRecord);
}
