#pragma once

#include "FW_IGame.h"
#include <FW_Renderer.h>
#include "FW_Vector3.h"
#include <FW_Math.h>
#include <FW_Time.h>

#include "Structs.h"
#include "Collision.h"
#include <FW_Threading.h>

namespace std
{
	class thread;
}

class Raytracer : public FW_IGame
{
public:
	void OnStartup() override;
	void OnShutdown() override;
	bool Run() override;

	void BuildGameImguiEditor(unsigned int aGameOffscreenBufferTextureID) override;

	const char* GetGameName() override { return "Raytracer"; }
	const char* GetDataFolderName() override { return "Raytracer"; }

private:
	enum RendererState
	{
		IDLE,
		START_RENDERING,
		RENDERING_IMAGE,
	};

	struct RenderingParameters
	{
		int myImageWidth;
		int mySamplesPerPixel;
		int myMaxBounces;
		int myNumberOfThreads;

		Vector3f myBackgroundColor;
		bool myUseFlatBackground;

		//int mySceneToRender;
	};

	Vector3f CastRay(Ray& aRay, const CollisionWorld& aWorld, int aDepth);

	void ClearImage();
	void UpdatePixelsInRow(int aRowNumber);
	void UpdatePixelsInRow(int aRowNumber, FW_GrowingArray<unsigned int>& aOutPixelArray);
	void PrintEntireImage();

	void TryUpdateRenderTexture();
	bool TryToFinalizeTexture();

	void ThreadUpdateFunction();
	void StopAllThreads();

	void UpdateStartRenderingState();
	void UpdateRenderingState();

	void BuildIdleStateUI();
	void BuildRenderingStateUI();


	int PackColor(const Vector3f& aColor, int aSamplesPerPixels);
	int PackColor(const Vector3f& aColor);
	FW_Renderer::Texture myTexture;
	FW_GrowingArray<unsigned int> myTexturePixels;

	Camera myCamera;
	volatile int myRowToCalculate;

	CollisionWorld myWorld;
	FW_Time::TimeUnit myStartTime;
	FW_Time::TimeUnit myLastRenderTextureUpdateTime;

	FW_Mutex myRenderTextureMutex;
	FW_GrowingArray<std::thread*> myTextureBuildingThreads;
	volatile bool myShouldStopThreads = false;
	volatile int myRunningThreadCount = 0;

	RendererState myCurrentState;
	RenderingParameters myRenderingParameters;
};
