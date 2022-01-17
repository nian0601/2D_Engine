#include "Raytracer.h"
#include "FW_Includes.h"
#include <FW_Math.h>
#include <SFML_Renderer.h>
#include <FW_Logger.h>
#include <thread>

#include "Materials.h"

void Raytracer::OnStartup()
{
	myRenderingParameters.mySamplesPerPixel = 16;
	myRenderingParameters.myMaxBounces = 8;
	myRenderingParameters.myImageWidth = 480;
	myRenderingParameters.myNumberOfThreads = 4;

	// Should be able to select Scene from UI
	//BuildRandomScene();
	BuildSimpleLightScene();

	myCurrentState = RendererState::IDLE;
}

void Raytracer::OnShutdown()
{
	StopAllThreads();
}

bool Raytracer::Run()
{
	switch (myCurrentState)
	{
	case Raytracer::IDLE:
		// Doesnt need to do much?
		// Handle UI in ImGUI-fuction
		break;
	case Raytracer::START_RENDERING:
		UpdateStartRenderingState();
		break;
	case Raytracer::RENDERING_IMAGE:
		UpdateRenderingState();
		break;
	default:
		break;
	}

	if (myTexture.myTextureID != -1)
		FW_Renderer::RenderTexture(myTexture, { 0, 0 });

	return true;
}

void Raytracer::BuildGameImguiEditor(unsigned int aGameOffscreenBufferTextureID)
{
	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)55), ImVec2(350, 400), false);
	
	static bool showDemoWindow = false;
	ImGui::Checkbox("Demo Window", &showDemoWindow);
	ImGui::ShowDemoWindow(&showDemoWindow);
	ImGui::Separator();

	switch (myCurrentState)
	{
	case Raytracer::IDLE:
	{
		BuildIdleStateUI();
		break;
	}
	case Raytracer::RENDERING_IMAGE:
	{
		BuildRenderingStateUI();
		break;
	}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	const float aspectRatio = 16.f / 9.f;
	const int imageWidth = 720;
	const int imageHeight = static_cast<int>(imageWidth / aspectRatio);
	ImGui::Image(aGameOffscreenBufferTextureID, ImVec2(imageWidth, imageHeight), ImVec2(0, 1), ImVec2(1, 0));
}

void Raytracer::BuildRandomScene()
{
	myRenderingParameters.myBackgroundColor = { 0.f, 0.f, 0.f };
	myRenderingParameters.myUseFlatBackground = false;
	myRenderingParameters.myLookFrom = { 13.f, 2.f, -3.f };
	myRenderingParameters.myLookAt = { 0.f, 1.f, 0.f };
	myRenderingParameters.myUp = { 0.f, 1.f, 0.f };
	myRenderingParameters.myAperature = 0.1f;
	myRenderingParameters.myDistToFocus = 10.f;
	myRenderingParameters.myVFov = 20.f;


	myWorld.ClearWorld();

	Sphere ground;
	ground.myPosition = { 0.f, -1000.f, 0.f };
	ground.myRadius = 1000.f;
	ground.myMaterial.myMaterialType = Material::Lambertian;
	ground.myMaterial.myColor = { 0.5f, 0.5f, 0.5f };
	ground.myUIName = "Ground";
	myWorld.AddObject(ground);

	Sphere dialectric;
	dialectric.myRadius = 1.f;
	dialectric.myPosition = { 0.f, 1.f, 0.f };
	dialectric.myMaterial.myMaterialType = Material::Dialectric;
	dialectric.myMaterial.myMaterialParameter = 1.5f;
	dialectric.myUIName = "Dialectric";
	myWorld.AddObject(dialectric);

	Sphere lambertian;
	lambertian.myRadius = 1.f;
	lambertian.myPosition = { -4.f, 1.f, 0.f };
	lambertian.myMaterial.myMaterialType = Material::Lambertian;
	lambertian.myMaterial.myColor = { 0.4f, 0.2f, 0.1f };
	lambertian.myUIName = "Lambertian";
	myWorld.AddObject(lambertian);

	Sphere metal;
	metal.myRadius = 1.f;
	metal.myPosition = { 4.f, 1.f, 0.f };
	metal.myMaterial.myMaterialType = Material::Metal;
	metal.myMaterial.myMaterialParameter = 0.f;
	metal.myMaterial.myColor = { 0.7f, 0.6f, 0.5f };
	metal.myUIName = "Metal";
	myWorld.AddObject(metal);

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			float chooseMaterial = FW_RandFloat();
			Vector3f center = { a + 0.9f * FW_RandFloat(), 0.2f, b + 0.9f * FW_RandFloat() };

			Vector3f vectorForDistanceTest = center - Vector3f(4.f, 0.2f, 0.f);
			if (Length(vectorForDistanceTest) > 0.9f)
			{
				Material material;

				if (chooseMaterial < 0.8f)
				{
					material.myMaterialType = Material::Lambertian;
					material.myColor = FW_RandomVector3f() * FW_RandomVector3f();
				}
				else if (chooseMaterial < 0.95f)
				{
					material.myMaterialType = Material::Metal;
					material.myMaterialParameter = FW_RandFloat(0.f, 0.5f);
					material.myColor = FW_RandomVector3f(0.5f, 1.f);
				}
				else
				{
					material.myMaterialType = Material::Dialectric;
					material.myMaterialParameter = 1.5f;
				}

				Sphere sphere;
				sphere.myRadius = 0.2f;
				sphere.myMaterial = material;
				sphere.myPosition = center;
				myWorld.AddObject(sphere);
			}
		}
	}
}

void Raytracer::BuildSimpleLightScene()
{
	myRenderingParameters.myBackgroundColor = { 0.f, 0.f, 0.f };
	myRenderingParameters.myUseFlatBackground = true;
	myRenderingParameters.myLookFrom = { 26.f, 3.f, 6.f };
	myRenderingParameters.myLookAt = { 0.f, 2.f, 0.f };
	myRenderingParameters.myUp = { 0.f, 1.f, 0.f };
	myRenderingParameters.myAperature = 0.1f;
	myRenderingParameters.myDistToFocus = 10.f;
	myRenderingParameters.myVFov = 20.f;

	myRenderingParameters.mySamplesPerPixel = 100;
	myRenderingParameters.myMaxBounces = 50;

	myWorld.ClearWorld();

	Sphere ground;
	ground.myPosition = { 0.f, -1000.f, 0.f };
	ground.myRadius = 1000.f;
	ground.myMaterial.myMaterialType = Material::Lambertian;
	ground.myMaterial.myColor = { 0.5f, 0.5f, 0.5f };
	ground.myUIName = "Ground";
	myWorld.AddObject(ground);

	Sphere lambertian;
	lambertian.myRadius = 2.f;
	lambertian.myPosition = { 0.f, 2.f, 0.f };
	lambertian.myMaterial.myMaterialType = Material::Lambertian;
	lambertian.myMaterial.myColor = { 0.4f, 0.2f, 0.1f };
	lambertian.myUIName = "Lambertian";
	myWorld.AddObject(lambertian);

	XY_Rect diffLight;
	diffLight.x0 = 3;
	diffLight.x1 = 5;
	diffLight.y0 = 1;
	diffLight.y1 = 3;
	diffLight.k = -2;
	diffLight.myMaterial.myMaterialType = Material::LightSource;
	diffLight.myMaterial.myColor = { 4.f, 4.f, 4.f };
	myWorld.AddObject(diffLight);


	Sphere lightSphere;
	lightSphere.myRadius = 2.f;
	lightSphere.myPosition = { 2.f, 2.f, 6.f };
	lightSphere.myMaterial.myMaterialType = Material::LightSource;
	lightSphere.myMaterial.myColor = { 4.f, 4.f, 4.f };
	lightSphere.myUIName = "LightSphere";
	myWorld.AddObject(lightSphere);
}

Vector3f Raytracer::CastRay(Ray& aRay, const CollisionWorld& aWorld, int aDepth)
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

void Raytracer::ClearImage()
{
	for (int y = 0; y < myTexture.mySize.y; ++y)
	{
		for (int x = 0; x < myTexture.mySize.x; ++x)
		{
			int pixelIndex = y * myTexture.mySize.x + x;
			myTexturePixels[pixelIndex] = PackColor({ 1.f, 1.f, 1.f });
		}
	}
	FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());
}

void Raytracer::UpdatePixelsInRow(int aRowNumber)
{
	for (int x = 0; x < myTexture.mySize.x; ++x)
	{
		Vector3f pixelColor;
		for (int i = 0; i < myRenderingParameters.mySamplesPerPixel; ++i)
		{
			float u = float(x + FW_RandFloat()) / (myTexture.mySize.x - 1);
			float v = float(aRowNumber + FW_RandFloat()) / (myTexture.mySize.y - 1);

			Ray ray = myCamera.GetRay(u, v);
			pixelColor += CastRay(ray, myWorld, myRenderingParameters.myMaxBounces);
		}

		int pixelIndex = aRowNumber * myTexture.mySize.x + x;
		myTexturePixels[pixelIndex] = PackColor(pixelColor, myRenderingParameters.mySamplesPerPixel);
	}
}

void Raytracer::UpdatePixelsInRow(int aRowNumber, FW_GrowingArray<unsigned int>& aOutPixelArray)
{
	for (int x = 0; x < myTexture.mySize.x; ++x)
	{
		Vector3f pixelColor;
		for (int i = 0; i < myRenderingParameters.mySamplesPerPixel; ++i)
		{
			if (myShouldStopThreads)
				return;

			float u = float(x + FW_RandFloat()) / (myTexture.mySize.x - 1);
			float v = float(aRowNumber + FW_RandFloat()) / (myTexture.mySize.y - 1);

			Ray ray = myCamera.GetRay(u, v);
			pixelColor += CastRay(ray, myWorld, myRenderingParameters.myMaxBounces);
		}

		aOutPixelArray[x] = PackColor(pixelColor, myRenderingParameters.mySamplesPerPixel);
	}
}

void Raytracer::PrintEntireImage()
{
	for (int y = 0; y < myTexture.mySize.y; ++y)
		UpdatePixelsInRow(y);

	FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());
}

void Raytracer::TryUpdateRenderTexture()
{
	FW_Time::TimeUnit currentTime = FW_Time::GetTime();
	float duration = FW_Time::ConvertTimeUnitToGameTime(currentTime - myLastRenderTextureUpdateTime);

	if (duration >= 1.f)
	{
		FW_ReadLock lock(myRenderTextureMutex);

		myLastRenderTextureUpdateTime = FW_Time::GetTime();
		FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());
	}
}

bool Raytracer::TryToFinalizeTexture()
{
	if (myRunningThreadCount <= 0)
	{
		FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());

		FW_Time::TimeUnit endTime = FW_Time::GetTime();
		float duration = FW_Time::ConvertTimeUnitToGameTime(endTime - myStartTime);

		FW_String message = "It took ";
		message += duration;
		message += "s to render";
		FW_Logger::AddMessage(message.GetBuffer());

		StopAllThreads();
		return true;
	}

	return false;
}

void Raytracer::ThreadUpdateFunction()
{
	++myRunningThreadCount;
	FW_GrowingArray<unsigned int> temporaryPixels;
	for (int x = 0; x < myTexture.mySize.x; ++x)
		temporaryPixels.Add(0);

	int rowToUpdate = myRowToCalculate++;
	while (rowToUpdate < myTexture.mySize.y && myShouldStopThreads == false)
	{
		UpdatePixelsInRow(rowToUpdate, temporaryPixels);

		{
			FW_ReadWriteLock lock(myRenderTextureMutex);
			for (int x = 0; x < myTexture.mySize.x; ++x)
			{
				int pixelIndex = rowToUpdate * myTexture.mySize.x + x;
				myTexturePixels[pixelIndex] = temporaryPixels[x];
			}
		}

		rowToUpdate = myRowToCalculate++;
	}

	--myRunningThreadCount;
}

void Raytracer::StopAllThreads()
{
	myShouldStopThreads = true;

	for (std::thread* thread : myTextureBuildingThreads)
		thread->join();

	myTextureBuildingThreads.DeleteAll();
}

void Raytracer::UpdateStartRenderingState()
{
	// All of this should come from UI
	const float aspectRatio = 16.f / 9.f;
	const int imageWidth = myRenderingParameters.myImageWidth;
	const int imageHeight = static_cast<int>(imageWidth / aspectRatio);
	SFML_Renderer::ResizeOffscreenBuffer(imageWidth, aspectRatio);

	FW_Renderer::DeleteTexture(myTexture);
	myTexture = FW_Renderer::CreateTexture({ imageWidth, imageHeight });

	myTexturePixels.RemoveAll();
	myTexturePixels.Reserve(myTexture.mySize.x * myTexture.mySize.y);

	ClearImage();

	// Camera parameters should also be configurable through UI
	myCamera.Setup(
		myRenderingParameters.myLookFrom,
		myRenderingParameters.myLookAt,
		myRenderingParameters.myUp,
		myRenderingParameters.myVFov,
		aspectRatio,
		myRenderingParameters.myAperature,
		myRenderingParameters.myDistToFocus);

	myWorld.ClearOctree();
	myWorld.BuildOctree();

	myRowToCalculate = 0;
	myStartTime = FW_Time::GetTime();
	myLastRenderTextureUpdateTime = myStartTime;

	myShouldStopThreads = false;
	myRunningThreadCount = 0;
	myTextureBuildingThreads.DeleteAll();
	for (int i = 0; i < myRenderingParameters.myNumberOfThreads; ++i)
	{
		myTextureBuildingThreads.Add(new std::thread([this]() {ThreadUpdateFunction(); }));
	}

	myCurrentState = RendererState::RENDERING_IMAGE;
}

void Raytracer::UpdateRenderingState()
{
	TryUpdateRenderTexture();

	if (TryToFinalizeTexture())
	{
		myCurrentState = RendererState::IDLE;
	}
}

void Raytracer::BuildIdleStateUI()
{
	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)456), ImVec2(0, -60), false);

	if (ImGui::CollapsingHeader("Render Settings"))
	{
		if (ImGui::Button("Quick Iterations"))
		{
			myRenderingParameters.mySamplesPerPixel = 16;
			myRenderingParameters.myMaxBounces = 8;
			myRenderingParameters.myImageWidth = 480;
			myRenderingParameters.myNumberOfThreads = 4;
		}

		if (ImGui::Button("Decent Quality"))
		{
			myRenderingParameters.myImageWidth = 720;
			myRenderingParameters.mySamplesPerPixel = 100;
			myRenderingParameters.myMaxBounces = 16;
			myRenderingParameters.myNumberOfThreads = 4;
		}

		if (ImGui::Button("High Quality"))
		{
			myRenderingParameters.myImageWidth = 1920;
			myRenderingParameters.mySamplesPerPixel = 100;
			myRenderingParameters.myMaxBounces = 16;
			myRenderingParameters.myNumberOfThreads = 4;
		}

		if (ImGui::Button("Highest Quality"))
		{
			myRenderingParameters.myImageWidth = 2560;
			myRenderingParameters.mySamplesPerPixel = 100;
			myRenderingParameters.myMaxBounces = 16;
			myRenderingParameters.myNumberOfThreads = 4;
		}

		ImGui::Separator();

		ImGui::DragInt("Image Width", &myRenderingParameters.myImageWidth, 1, 1, 2560);
		ImGui::DragInt("Samples per Pixel", &myRenderingParameters.mySamplesPerPixel, 1, 1, 10000);
		ImGui::DragInt("Max Bounces", &myRenderingParameters.myMaxBounces, 1, 1, 1000);
		ImGui::DragInt("Threads", &myRenderingParameters.myNumberOfThreads, 1, 1, 4);
	}
	if (ImGui::CollapsingHeader("World Settings"))
	{
		ImGui::DragFloat3("Background Color", &myRenderingParameters.myBackgroundColor.x, 0.01f, 0.f, 1.f);
		ImGui::Checkbox("Flat Background", &myRenderingParameters.myUseFlatBackground);

		ImGui::DragFloat3("Camera Position", &myRenderingParameters.myLookFrom.x, 0.1f, -50.f, 50.f);

		if (ImGui::TreeNode("Objects"))
		{
			ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)87), ImVec2(-10, 200), true);

			const char* materialTypes[] = { "Lambertian", "Metallic", "Dialectrict", "Light" };

			FW_GrowingArray<Sphere>& spheres = myWorld.GetAllSpheres();
			char sphereLable[64];
			for (int i = 0; i < spheres.Count(); ++i)
			{
				Sphere& sphere = spheres[i];

				if(sphere.myUIName.Empty())
					sprintf_s(sphereLable, 64, "Sphere %i", i);
				else
					sprintf_s(sphereLable, 64, "%s", sphere.myUIName.GetBuffer());

				if (ImGui::TreeNode(sphereLable))
				{
					ImGui::DragFloat3("Position", &sphere.myPosition.x, 0.1f, -10.f, 10.f);
					ImGui::DragFloat3("Color", &sphere.myMaterial.myColor.x, 0.1f, 0.f, 4.f);
					ImGui::DragFloat("Radius", &sphere.myRadius, 1, 0.f, 100.f);
					ImGui::Combo("Material", &sphere.myMaterial.myMaterialType, materialTypes, IM_ARRAYSIZE(materialTypes));

					ImGui::DragFloat("Material Parameter", &sphere.myMaterial.myMaterialParameter, 1, 0.f, 4.f);

					ImGui::TreePop();
				}
			}

			ImGui::EndChild();

			ImGui::TreePop();
		}

		if (ImGui::Button("Add Sphere"))
		{
			Sphere sphere;
			sphere.myPosition = { 0.f, 0.f, 0.f };
			sphere.myRadius = 0.5f;
			sphere.myMaterial.myMaterialType = Material::MaterialType::Lambertian;
			sphere.myMaterial.myMaterialParameter = 0.f;
			sphere.myMaterial.myColor = { 1.f, 0.f, 0.f };

			myWorld.AddObject(sphere);
		}

		if (ImGui::Button("Remove All"))
		{
			myWorld.ClearWorld();
		}

		ImGui::Spacing();

		if (ImGui::Button("Load Random World"))
		{
			BuildRandomScene();
		}
	}

	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Start Render"))
		myCurrentState = RendererState::START_RENDERING;

	if (ImGui::Button("Save Result to File"))
		FW_Renderer::SaveTextureToFile(myTexture, "test.png");
}

void Raytracer::BuildRenderingStateUI()
{
	if (ImGui::Button("Stop Render"))
	{
		StopAllThreads();
		myCurrentState = RendererState::IDLE;
	}
}

int Raytracer::PackColor(const Vector3f& aColor, int aSamplesPerPixels)
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

int Raytracer::PackColor(const Vector3f& aColor)
{
	int color = 0;
	color |= unsigned char(aColor.x * 255.99f) << 0;
	color |= unsigned char(aColor.y * 255.99f) << 8;
	color |= unsigned char(aColor.z * 255.99f) << 16;
	color |= 255 << 24;
	return color;
}

