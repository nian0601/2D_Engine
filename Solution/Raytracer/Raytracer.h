#pragma once

#include "FW_IGame.h"
#include <FW_Renderer.h>
#include "FW_Vector3.h"
#include <FW_Math.h>
#include <FW_Time.h>
#include <FW_Threading.h>

#include "Structs.h"
#include "Collision.h"
#include "ImageRenderer.h"

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
	void BuildIdleStateUI();
	void BuildRenderingImageStateUI();
	void BuildRenderingVideoStateUI();


	const char* GetGameName() override { return "Raytracer"; }
	const char* GetDataFolderName() override { return "Raytracer"; }

private:
	enum RendererState
	{
		IDLE,
		START_RENDERING_IMAGE,
		RENDERING_IMAGE,
		START_RENDERING_VIDEO,
		RENDERING_VIDEO,
	};

	void BuildRandomScene();
	void BuildSimpleLightScene();
	void BuildCornellBoxScene();
	void BuildFinalBoxScene();
	void BuildRandomSceneV2();

	void UpdateStartRenderingImageState();
	void UpdateRenderingImageState();

	void UpdateStartRenderingVideoState();
	void UpdateRenderingVideoState();

	void PrepareTextureForRender();
	void TryUpdateRenderTexture();
	bool TryToFinalizeTexture();

	void SaveSceneToDisk();
	void LoadSceneFromDisk();

	FW_Renderer::Texture myTexture;
	FW_GrowingArray<unsigned int> myTexturePixels;
	FW_Time::TimeUnit myLastRenderTextureUpdateTime;

	CollisionWorld myWorld;
	Camera myCamera;
	RenderingParameters myRenderingParameters;

	RendererState myCurrentState;
	ImageRenderer* myImageRenderer;

	int myVideoFrameToRender;
	int myNumVideoFramesToRender = 10;
	int myFPS = 24;
	float myDeltaTime = 1.f / 24.f;
	float myAverageTimePerVideoFrame = 1.f;
	FW_Time::TimeUnit myVideoRenderStartTime;
};
