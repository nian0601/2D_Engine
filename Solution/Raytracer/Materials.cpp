#include "Materials.h"

namespace Materials_Private
{
	float Reflectance(float aCosine, float aRefIdx)
	{
		// Schlick's approximation for reflectance
		float r0 = (1.f - aRefIdx) / (1.f + aRefIdx);
		r0 = r0 * r0;
		return r0 + (1.f - r0) * pow((1 - aCosine), 5);
	}

	bool NearZero(const Vector3f& aVector)
	{
		const float s = float(1e-8);
		return (fabs(aVector.x) < s) && (fabs(aVector.y) < s) && (fabs(aVector.z) < s);
	}

	bool ScatterLambertian(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay)
	{
		Vector3f scatterDirection = aHitRecord.myNormal + FW_RandomUnitVector();
		if (NearZero(scatterDirection))
			scatterDirection = aHitRecord.myNormal;

		aScatteredRay = { aHitRecord.myPosition, scatterDirection };
		anAttenuation = aHitRecord.myMaterial.myColor * aHitRecord.myMaterial.myInterpolator.myValue;
		return true;
	}

	bool ScatterMetal(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay)
	{
		Vector3f reflected = ReflectAround(GetNormalized(aRayIn.myDirection), aHitRecord.myNormal);
		aScatteredRay = { aHitRecord.myPosition, reflected + aHitRecord.myMaterial.myMaterialParameter * FW_RandomUnitSphereVector() };
		anAttenuation = aHitRecord.myMaterial.myColor * aHitRecord.myMaterial.myInterpolator.myValue;
		return Dot(aScatteredRay.myDirection, aHitRecord.myNormal) > 0.f;
	}

	bool ScatterDialectric(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay)
	{
		anAttenuation = { 1.f, 1.f, 1.f };

		float refractionRatio = aHitRecord.myIsFrontFace ? (1.f / aHitRecord.myMaterial.myMaterialParameter) : aHitRecord.myMaterial.myMaterialParameter;

		Vector3f unitDirection = GetNormalized(aRayIn.myDirection);

		float cosTheta = fmin(Dot(-unitDirection, aHitRecord.myNormal), 1.f);
		float sinTheta = sqrt(1.f - cosTheta * cosTheta);

		bool cannotRefract = refractionRatio * sinTheta > 1.f;
		Vector3f direction;
		if (cannotRefract || Reflectance(cosTheta, refractionRatio) > FW_RandFloat())
			direction = ReflectAround(unitDirection, aHitRecord.myNormal);
		else
			direction = Refract(unitDirection, aHitRecord.myNormal, refractionRatio);;

		aScatteredRay = { aHitRecord.myPosition, direction };

		return true;
	}

	bool ScatterLightSource(const Ray& aRayIn, const RayHit& aHitRecord)
	{
		return false;
	}
}

namespace Materials
{
	bool ScatterRay(const Ray& aRayIn, const RayHit& aHitRecord, Vector3f& anAttenuation, Ray& aScatteredRay)
	{
		switch (aHitRecord.myMaterial.myMaterialType)
		{
		case Material::Lambertian:
			return Materials_Private::ScatterLambertian(aRayIn, aHitRecord, anAttenuation, aScatteredRay);
		case Material::Metal:
			return Materials_Private::ScatterMetal(aRayIn, aHitRecord, anAttenuation, aScatteredRay);
		case Material::Dialectric:
			return Materials_Private::ScatterDialectric(aRayIn, aHitRecord, anAttenuation, aScatteredRay);
		case Material::LightSource:
			return Materials_Private::ScatterLightSource(aRayIn, aHitRecord);
		}

		return false;
	}

	Vector3f EmitFromHit(const RayHit& aHitRecord)
	{
		if (aHitRecord.myMaterial.myMaterialType == Material::LightSource)
		{
			return aHitRecord.myMaterial.myColor * aHitRecord.myMaterial.myInterpolator.myValue;
		}
		else
		{
			return { 0.f, 0.f, 0.f };
		}
	}

}