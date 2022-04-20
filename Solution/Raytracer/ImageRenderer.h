#pragma once

#include "Structs.h"
#include "Collision.h"

#include <FW_Math.h>
#include <FW_Time.h>
#include <FW_Threading.h>

class ImageRenderer
{
public:
	ImageRenderer(const Camera& aCamera, const CollisionWorld& aWorld, const RenderingParameters& someRenderingParameters, int aImageWidth, int aImageHeight);
	~ImageRenderer();

	bool IsFinished() const;
	void CopyCurrentImageState(FW_GrowingArray<unsigned int>& aOutPixelArray);
	FW_Time::TimeUnit GetStartTime() const { return myStartTime; }

private:
	void ThreadUpdateFunction();
	void StopAllThreads();

	Vector3f CastRay(Ray& aRay, const CollisionWorld& aWorld, int aDepth);
	void UpdatePixelsInRow(int aRowNumber, FW_GrowingArray<unsigned int>& aOutPixelArray);
	int PackColor(const Vector3f& aColor, int aSamplesPerPixels);

	const Camera& myCamera;
	const CollisionWorld& myWorld;
	const RenderingParameters& myRenderingParameters;
	const Vector2i myImageSize;

	FW_GrowingArray<unsigned int> myTexturePixels;
	volatile int myRowToCalculate;

	FW_Time::TimeUnit myStartTime;

	FW_Mutex myRenderTextureMutex;
	FW_GrowingArray<std::thread*> myTextureBuildingThreads;
	volatile bool myShouldStopThreads = false;
	volatile int myRunningThreadCount = 0;

};
