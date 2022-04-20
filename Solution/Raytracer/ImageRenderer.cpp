#include "ImageRenderer.h"
#include "Materials.h"

ImageRenderer::ImageRenderer(const Camera& aCamera, const CollisionWorld& aWorld, const RenderingParameters& someRenderingParameters, int aImageWidth, int aImageHeight)
	: myCamera(aCamera)
	, myWorld(aWorld)
	, myRenderingParameters(someRenderingParameters)
	, myImageSize(aImageWidth, aImageHeight)
	, myRowToCalculate(0)
	, myShouldStopThreads(false)
	, myStartTime(FW_Time::GetTime())
{
	myTexturePixels.Reserve(myImageSize.x * myImageSize.y);
	for (int y = 0; y < myImageSize.y; ++y)
	{
		for (int x = 0; x < myImageSize.x; ++x)
		{
			int pixelIndex = y * myImageSize.x + x;
			myTexturePixels[pixelIndex] = PackColor({ 1.f, 1.f, 1.f }, 1);
		}
	}

	for (int i = 0; i < myRenderingParameters.myNumberOfThreads; ++i)
		myTextureBuildingThreads.Add(new std::thread([this]() {ThreadUpdateFunction(); }));
}

ImageRenderer::~ImageRenderer()
{
	StopAllThreads();
}

bool ImageRenderer::IsFinished() const
{
	return myRunningThreadCount <= 0;
}

void ImageRenderer::CopyCurrentImageState(FW_GrowingArray<unsigned int>& aOutPixelArray)
{
	FW_ReadLock lock(myRenderTextureMutex);

	if (myTexturePixels.Count() != aOutPixelArray.Count())
		return;

	aOutPixelArray = myTexturePixels;
}

void ImageRenderer::ThreadUpdateFunction()
{
	++myRunningThreadCount;
	FW_GrowingArray<unsigned int> temporaryPixels;
	for (int x = 0; x < myImageSize.x; ++x)
		temporaryPixels.Add(0);

	int rowToUpdate = myRowToCalculate++;
	while (rowToUpdate < myImageSize.y && myShouldStopThreads == false)
	{
		UpdatePixelsInRow(rowToUpdate, temporaryPixels);

		{
			FW_ReadWriteLock lock(myRenderTextureMutex);
			for (int x = 0; x < myImageSize.x; ++x)
			{
				int pixelIndex = rowToUpdate * myImageSize.x + x;
				myTexturePixels[pixelIndex] = temporaryPixels[x];
			}
		}

		rowToUpdate = myRowToCalculate++;
	}

	--myRunningThreadCount;
}

void ImageRenderer::StopAllThreads()
{
	myShouldStopThreads = true;

	for (std::thread* thread : myTextureBuildingThreads)
		thread->join();

	myTextureBuildingThreads.DeleteAll();
}

Vector3f ImageRenderer::CastRay(Ray& aRay, const CollisionWorld& aWorld, int aDepth)
{
	if (aDepth <= 0)
		return { 0.f, 0.f, 0.f };

	RayHit hitRecord;
	if (!aWorld.CastRay(aRay, hitRecord))
	{
		if (myRenderingParameters.myUseFlatBackground)
			return myRenderingParameters.myBackgroundColor;

		Vector3f dir = GetNormalized(aRay.myDirection);
		float t = 0.5f * (dir.y + 1.f);

		return (1.f - t) * Vector3f(1.f, 1.f, 1.f) + t * Vector3f(0.5f, 0.7f, 1.f);
	}

	Ray scatteredRay;
	Vector3f attenuation;
	Vector3f emitted = Materials::EmitFromHit(hitRecord);

	if (!Materials::ScatterRay(aRay, hitRecord, attenuation, scatteredRay))
		return emitted;

	return emitted + attenuation * CastRay(scatteredRay, aWorld, aDepth - 1);
}

void ImageRenderer::UpdatePixelsInRow(int aRowNumber, FW_GrowingArray<unsigned int>& aOutPixelArray)
{
	for (int x = 0; x < myImageSize.x; ++x)
	{
		Vector3f pixelColor;
		for (int i = 0; i < myRenderingParameters.mySamplesPerPixel; ++i)
		{
			if (myShouldStopThreads)
				return;

			float u = float(x + FW_RandFloat()) / (myImageSize.x - 1);
			float v = float(aRowNumber + FW_RandFloat()) / (myImageSize.y - 1);

			Ray ray = myCamera.GetRay(u, v);
			pixelColor += CastRay(ray, myWorld, myRenderingParameters.myMaxBounces);
		}

		aOutPixelArray[x] = PackColor(pixelColor, myRenderingParameters.mySamplesPerPixel);
	}
}

int ImageRenderer::PackColor(const Vector3f& aColor, int aSamplesPerPixels)
{
	const float scale = 1.f / aSamplesPerPixels;
	const float r = sqrt(aColor.x * scale);
	const float g = sqrt(aColor.y * scale);
	const float b = sqrt(aColor.z * scale);

	int color = 0;
	color |= unsigned char(FW_Clamp(r, 0.f, 0.999f) * 255.99f) << 0;
	color |= unsigned char(FW_Clamp(g, 0.f, 0.999f) * 255.99f) << 8;
	color |= unsigned char(FW_Clamp(b, 0.f, 0.999f) * 255.99f) << 16;
	color |= 255 << 24;
	return color;
}

