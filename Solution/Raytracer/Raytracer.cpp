#include "Raytracer.h"
#include "FW_Includes.h"
#include <FW_Math.h>
#include <SFML_Renderer.h>
#include <FW_Logger.h>
#include <thread>

#include "Materials.h"
#include "FW_FileProcessor.h"

void Raytracer::OnStartup()
{
	myImageRenderer = nullptr;

	myRenderingParameters.mySamplesPerPixel = 16;
	myRenderingParameters.myMaxBounces = 8;
	myRenderingParameters.myImageWidth = 480;
	myRenderingParameters.myNumberOfThreads = 4;

	// Should be able to select Scene from UI
	BuildRandomScene();
	//BuildSimpleLightScene();
	//BuildCornellBoxScene();
	//BuildFinalBoxScene();
	//BuildRandomSceneV2();
	//LoadSceneFromDisk();

	myCurrentState = RendererState::IDLE;
}

void Raytracer::OnShutdown()
{
	delete myImageRenderer;
	myImageRenderer = nullptr;
}

bool Raytracer::Run()
{
	switch (myCurrentState)
	{
	case Raytracer::IDLE:
		// Doesnt need to do much?
		// Handle UI in ImGUI-fuction
		break;
	case Raytracer::START_RENDERING_IMAGE:
		UpdateStartRenderingImageState();
		break;
	case Raytracer::RENDERING_IMAGE:
		UpdateRenderingImageState();
		break;
	case Raytracer::START_RENDERING_VIDEO:
		UpdateStartRenderingVideoState();
		break;
	case RendererState::RENDERING_VIDEO:
		UpdateRenderingVideoState();
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
	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)55), ImVec2(350, 500), false);
	
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
		BuildRenderingImageStateUI();
		break;
	}
	case Raytracer::RENDERING_VIDEO:
	{
		BuildRenderingVideoStateUI();
		break;
	}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)56), ImVec2(0, 500), false);

	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)57), ImVec2(0, 20), false);
	ImGui::Indent(30.f);
	if (ImGui::Button("Render Frame"))
	{

	}

	ImGui::SameLine();
	ImGui::Indent(200.f);
	if (ImGui::Button("Render Video"))
	{

	}
	ImGui::EndChild();


	const float imageWidth = 720.f;
	const float imageHeight = imageWidth / myRenderingParameters.myAspectRatio;
	ImGui::Image(aGameOffscreenBufferTextureID, ImVec2(imageWidth, imageHeight), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::EndChild();
}

void Raytracer::BuildIdleStateUI()
{
	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)456), ImVec2(0, -20), false);

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Render Settings"))
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

			if (ImGui::Button("Render Image"))
				myCurrentState = RendererState::START_RENDERING_IMAGE;

			if (ImGui::Button("Save Result to File"))
				FW_Renderer::SaveTextureToFile(myTexture, "test.png");

			ImGui::Separator();

			ImGui::DragInt("FPS", &myRenderingParameters.myFPS, 1, 1, 144);
			ImGui::DragFloat("Time", &myRenderingParameters.myVideoLenght, 0.1f, 0.1f, 60.f);

			if (ImGui::Button("Render Video"))
				myCurrentState = RendererState::START_RENDERING_VIDEO;

			ImGui::Separator();

			if (ImGui::Button("Save Scene To Disk"))
				SaveSceneToDisk();

			if (ImGui::Button("Load Scene From Disk"))
				LoadSceneFromDisk();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("World Settings"))
		{
			ImGui::DragFloat3("Background Color", &myRenderingParameters.myBackgroundColor.x, 0.01f, 0.f, 1.f);
			ImGui::Checkbox("Flat Background", &myRenderingParameters.myUseFlatBackground);

			ImGui::DragFloat3("Camera Position", &myRenderingParameters.myLookFrom.x, 0.1f, -50.f, 50.f);
			ImGui::DragFloat3("Focus Position", &myRenderingParameters.myLookAt.x, 0.1f, -50.f, 50.f);
			ImGui::DragFloat("Focus Distance", &myRenderingParameters.myDistToFocus, 0.1f, 0.1f, 100.f);

			if (ImGui::TreeNode("Objects"))
			{
				ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)87), ImVec2(-10, 200), true);

				const char* materialTypes[] = { "Lambertian", "Metallic", "Dialectrict", "Light" };

				FW_GrowingArray<Sphere>& spheres = myWorld.GetAllSpheres();
				char sphereLable[64];
				for (int i = 0; i < spheres.Count(); ++i)
				{
					Sphere& sphere = spheres[i];

					if (sphere.myUIName.Empty())
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
				BuildRandomScene();

			if (ImGui::Button("Render Image"))
				myCurrentState = RendererState::START_RENDERING_IMAGE;

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	/*if (ImGui::CollapsingHeader("Render Settings"))
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
	}*/

	/*if (ImGui::CollapsingHeader("World Settings"))
	{
		ImGui::DragFloat3("Background Color", &myRenderingParameters.myBackgroundColor.x, 0.01f, 0.f, 1.f);
		ImGui::Checkbox("Flat Background", &myRenderingParameters.myUseFlatBackground);

		ImGui::DragFloat3("Camera Position", &myRenderingParameters.myLookFrom.x, 0.1f, -50.f, 50.f);
		ImGui::DragFloat3("Focus Position", &myRenderingParameters.myLookAt.x, 0.1f, -50.f, 50.f);
		ImGui::DragFloat("Focus Distance", &myRenderingParameters.myDistToFocus, 0.1f, 0.1f, 100.f);

		if (ImGui::TreeNode("Objects"))
		{
			ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)87), ImVec2(-10, 200), true);

			const char* materialTypes[] = { "Lambertian", "Metallic", "Dialectrict", "Light" };

			FW_GrowingArray<Sphere>& spheres = myWorld.GetAllSpheres();
			char sphereLable[64];
			for (int i = 0; i < spheres.Count(); ++i)
			{
				Sphere& sphere = spheres[i];

				if (sphere.myUIName.Empty())
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
	}*/

	ImGui::EndChild();

	/*ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Render Image"))
		myCurrentState = RendererState::START_RENDERING_IMAGE;

	if (ImGui::Button("Save Result to File"))
		FW_Renderer::SaveTextureToFile(myTexture, "test.png");

	ImGui::Separator();

	ImGui::DragInt("FPS", &myRenderingParameters.myFPS, 1, 1, 144);
	ImGui::DragFloat("Time", &myRenderingParameters.myVideoLenght, 0.1f, 0.1f, 60.f);

	if (ImGui::Button("Render Video"))
		myCurrentState = RendererState::START_RENDERING_VIDEO;

	ImGui::Separator();

	if (ImGui::Button("Save Scene To Disk"))
		SaveSceneToDisk();

	if (ImGui::Button("Load Scene From Disk"))
		LoadSceneFromDisk();*/
}

void Raytracer::BuildRenderingImageStateUI()
{
	if (ImGui::Button("Stop Render"))
	{
		delete myImageRenderer;
		myImageRenderer = nullptr;

		myCurrentState = RendererState::IDLE;
	}
}

void Raytracer::BuildRenderingVideoStateUI()
{
	if (ImGui::Button("Stop Render"))
	{
		delete myImageRenderer;
		myImageRenderer = nullptr;

		myCurrentState = RendererState::IDLE;
	}

	float progress = myVideoFrameToRender / static_cast<float>(myNumVideoFramesToRender);
	float elapsedTime = FW_Time::ConvertTimeUnitToGameTime(FW_Time::GetTime() - myVideoRenderStartTime);

	ImGui::Text("%i/%i Finished (%.2f%%)", myVideoFrameToRender, myNumVideoFramesToRender, progress * 100.f);
	ImGui::Text("Time: %.1fs", elapsedTime);
	ImGui::Text("Estimated Remaining: %.1fs", (myAverageTimePerVideoFrame * myNumVideoFramesToRender) - elapsedTime);
}

//////////////////////////////////////////////////////////////////////////

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

	Sphere ground({ 0.f, -1000.f, 0.f }, 1000.f);
	ground.myMaterial = Material::MakeLambertian({ 0.5f, 0.5f, 0.5f });
	ground.myUIName = "Ground";
	myWorld.AddObject(ground);

	Sphere dialectric({ 0.f, 1.f, 0.f }, 1.f);
	dialectric.myMaterial = Material::MakeDialectric(1.5f);
	dialectric.myUIName = "Dialectric";
	myWorld.AddObject(dialectric);

	Sphere lambertian({ -4.f, 1.f, 0.f }, 1.f);
	lambertian.myMaterial = Material::MakeLambertian({ 0.4f, 0.2f, 0.1f });
	lambertian.myUIName = "Lambertian";
	myWorld.AddObject(lambertian);

	Sphere metal({ 4.f, 1.f, 0.f }, 1.f);
	metal.myMaterial = Material::MakeMetal({ 0.7f, 0.6f, 0.5f }, 0.f);
	metal.myUIName = "Metal";
	myWorld.AddObject(metal);

	int sphereCount = 1;
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
					material = Material::MakeLambertian(FW_RandomVector3f() * FW_RandomVector3f());
				}
				else if (chooseMaterial < 0.95f)
				{
					material = Material::MakeMetal(FW_RandomVector3f(0.5f, 1.f), FW_RandFloat(0.f, 0.5f));
				}
				else
				{
					material = Material::MakeDialectric(1.f);
				}

				Sphere sphere(center, 0.2f);
				sphere.myMaterial = material;

				sphere.myUIName = "Saved Sphere ";
				sphere.myUIName += sphereCount;
				myWorld.AddObject(sphere);
				++sphereCount;
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

	Sphere ground({ 0.f, -1000.f, 0.f }, 1000.f);
	ground.myMaterial = Material::MakeLambertian({ 0.5f, 0.5f, 0.5f });
	ground.myUIName = "Ground";
	myWorld.AddObject(ground);

	Sphere lambertian({ 0.f, 2.f, 0.f }, 2.f);
	lambertian.myMaterial = Material::MakeLambertian({ 0.4f, 0.2f, 0.1f });
	lambertian.myUIName = "Lambertian";
	myWorld.AddObject(lambertian);

	XY_Rect diffLight(3, 5, 1, 3, -2, Material::MakeLight({ 4.f, 4.f, 4.f }));
	myWorld.AddObject(diffLight);


	Sphere lightSphere({ 2.f, 2.f, 6.f }, 2.f);
	lightSphere.myMaterial = Material::MakeLight({ 4.f, 4.f, 4.f });
	lightSphere.myUIName = "LightSphere";
	myWorld.AddObject(lightSphere);
}

void Raytracer::BuildCornellBoxScene()
{
	myRenderingParameters.myBackgroundColor = { 0.f, 0.f, 0.f };
	myRenderingParameters.myUseFlatBackground = true;
	myRenderingParameters.myLookFrom = { 278.f, 278.f, -800.f };
	myRenderingParameters.myLookAt = { 278.f, 278.f, 0.f};
	myRenderingParameters.myVFov = 40.f;

	myRenderingParameters.mySamplesPerPixel = 200;
	myRenderingParameters.myMaxBounces = 50;
	myRenderingParameters.myImageWidth = 600;
	myRenderingParameters.myAspectRatio = 1.f;

	myWorld.ClearWorld();

	Material red = Material::MakeLambertian({ 0.65f, 0.05f, 0.05f });
	Material white = Material::MakeLambertian({ 0.73f, 0.73f, 0.73f });
	Material green = Material::MakeLambertian({0.12f, 0.45f, 0.15f});
	Material light = Material::MakeLight({ 15.f, 15.f, 15.f });

	YZ_Rect yz0(0.f, 555.f, 0.f, 555.f, 555.f, green);
	myWorld.AddObject(yz0);

	YZ_Rect yz1(0.f, 555.f, 0.f, 555.f, 0.f, red);
	myWorld.AddObject(yz1);

	XZ_Rect lightRect(213.f, 343.f, 227.f, 332.f, 554.f, light);
	myWorld.AddObject(lightRect);

	XZ_Rect xz0(0.f, 555.f, 0.f, 555.f, 0.f, white);
	myWorld.AddObject(xz0);

	XZ_Rect xz1(0.f, 555.f, 0.f, 555.f, 555.f, white);
	myWorld.AddObject(xz1);

	XY_Rect xy(0.f, 555.f, 0.f, 555.f, 555.f, white);
	myWorld.AddObject(xy);
	
	Box box0({ 130.f, 0.f, 65.f }, { 295.f, 165.f, 230.f }, white);
	myWorld.AddObject(box0);

	Box box1({ 265.f, 0.f, 295.f }, { 430.f, 330.f, 460.f }, white);
	myWorld.AddObject(box1);
}

void Raytracer::BuildFinalBoxScene()
{
	myRenderingParameters.myBackgroundColor = { 0.f, 0.f, 0.f };
	myRenderingParameters.myUseFlatBackground = true;
	myRenderingParameters.myLookFrom = { 478.f, 278.f, -600.f };
	myRenderingParameters.myLookAt = { 278.f, 278.f, 0.f };
	myRenderingParameters.myVFov = 40.f;

	myRenderingParameters.mySamplesPerPixel = 100;
	myRenderingParameters.myMaxBounces = 8;
	myRenderingParameters.myImageWidth = 800;
	myRenderingParameters.myAspectRatio = 1.f;

	myWorld.ClearWorld();


	Material ground = Material::MakeLambertian({ 0.48f, 0.83f, 0.53f });

	const int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++) 
	{
		for (int j = 0; j < boxes_per_side; j++) 
		{
			float w = 100.f;
			float x0 = -1000.f + i * w;
			float z0 = -1000.f + j * w;
			float y0 = 0.f;
			float x1 = x0 + w;
			float y1 = FW_RandFloat(1.f, 101.f);
			float z1 = z0 + w;

			myWorld.AddObject(Box({ x0, y0, z0 }, { x1, y1, z1 }, ground));
		}
	}

	Material light = Material::MakeLight({ 7.f, 7.f, 7.f });
	//myWorld.AddObject(XZ_Rect(123, 423.f, 147.f, 412.f, 554.f, light));
	myWorld.AddObject(XZ_Rect(123, 423.f, 147.f, 412.f, 354.f, light));
}

void Raytracer::BuildRandomSceneV2()
{
	myRenderingParameters.myBackgroundColor = { 0.f, 0.f, 0.f };
	myRenderingParameters.myUseFlatBackground = true;
	myRenderingParameters.myLookFrom = { 20.f, 10.f, -10.f };
	myRenderingParameters.myLookAt = { 0.f, 1.f, 0.f };
	myRenderingParameters.myUp = { 0.f, 1.f, 0.f };
	myRenderingParameters.myAperature = 0.1f;
	myRenderingParameters.myDistToFocus = 25.f;
	myRenderingParameters.myVFov = 20.f;


	myWorld.ClearWorld();

	Sphere ground({ 0.f, -1000.f, 0.f }, 1000.f);
	ground.myMaterial = Material::MakeLambertian({ 0.5f, 0.5f, 0.5f });
	ground.myUIName = "Ground";
	myWorld.AddObject(ground);

	Sphere dialectric({ 0.f, 1.f, 0.f }, 1.f);
	dialectric.myMaterial = Material::MakeDialectric(1.5f);
	dialectric.myUIName = "Dialectric";
	myWorld.AddObject(dialectric);

	Sphere lambertian({ -4.f, 1.f, 0.f }, 1.f);
	lambertian.myMaterial = Material::MakeLambertian({ 0.4f, 0.2f, 0.1f });
	lambertian.myUIName = "Lambertian";
	myWorld.AddObject(lambertian);

	Sphere metal({ 4.f, 1.f, 0.f }, 1.f);
	metal.myMaterial = Material::MakeMetal({ 0.7f, 0.6f, 0.5f }, 0.f);
	metal.myUIName = "Metal";
	myWorld.AddObject(metal);

	Sphere light({ 0.f, 10.f, 0.f }, 3.f);
	light.myMaterial = Material::MakeLight({ 1.f, 1.f, 1.f });
	light.myUIName = "SkyLight";
	myWorld.AddObject(light);

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

				if (chooseMaterial < 0.1f)
				{
					float intensity = FW_RandFloat(2.f, 4.f);
					Vector3f color = FW_RandomVector3f(0.3f, 1.f);
					material = Material::MakeLight(color * intensity);
				}
				else if (chooseMaterial < 0.8f)
				{
					material = Material::MakeLambertian(FW_RandomVector3f() * FW_RandomVector3f());
				}
				else if (chooseMaterial < 0.95f)
				{
					material = Material::MakeMetal(FW_RandomVector3f(0.5f, 1.f), FW_RandFloat(0.f, 0.5f));
				}
				else
				{
					material = Material::MakeDialectric(1.f);
				}

				Sphere sphere(center, 0.2f);
				sphere.myMaterial = material;
				myWorld.AddObject(sphere);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void Raytracer::UpdateStartRenderingImageState()
{
	PrepareTextureForRender();

	myCamera.Setup(
		myRenderingParameters.myLookFrom,
		myRenderingParameters.myLookAt,
		myRenderingParameters.myUp,
		myRenderingParameters.myVFov,
		myRenderingParameters.myAspectRatio,
		myRenderingParameters.myAperature,
		myRenderingParameters.myDistToFocus);

	myWorld.ClearOctree();
	myWorld.BuildOctree();

	myImageRenderer = new ImageRenderer(myCamera, myWorld, myRenderingParameters, myTexture.mySize.x, myTexture.mySize.y);
	myLastRenderTextureUpdateTime = myImageRenderer->GetStartTime();
	myCurrentState = RendererState::RENDERING_IMAGE;
}

void Raytracer::UpdateRenderingImageState()
{
	TryUpdateRenderTexture();

	if (TryToFinalizeTexture())
	{
		myCurrentState = RendererState::IDLE;
	}
}

void Raytracer::UpdateStartRenderingVideoState()
{
	PrepareTextureForRender();

	myVideoFrameToRender = 0;
	myDeltaTime = 1.f / myRenderingParameters.myFPS;
	myNumVideoFramesToRender = static_cast<int>(myRenderingParameters.myFPS * myRenderingParameters.myVideoLenght);
	myLastRenderTextureUpdateTime = FW_Time::GetTime();
	myVideoRenderStartTime = FW_Time::GetTime();
	myCurrentState = RendererState::RENDERING_VIDEO;

	myWorld.ClearOctree();
	myWorld.BuildOctree();
}

void Raytracer::UpdateRenderingVideoState()
{
	// If we have a ImageRenderer that means we're currently rendering a frame
	if (myImageRenderer)
	{
		TryUpdateRenderTexture();

		if (TryToFinalizeTexture())
		{
			FW_String filename = "Video/frame";
			filename += myVideoFrameToRender;
			filename += ".png";
			FW_Renderer::SaveTextureToFile(myTexture, filename.GetRawBuffer());
			++myVideoFrameToRender;

			float elapsedTime = FW_Time::ConvertTimeUnitToGameTime(FW_Time::GetTime() - myVideoRenderStartTime);
			myAverageTimePerVideoFrame = elapsedTime / myVideoFrameToRender;
		}
	}
	else
	{
		if (myVideoFrameToRender >= myNumVideoFramesToRender)
		{
			myCurrentState = RendererState::IDLE;
		}
		else
		{
			myRenderingParameters.myLookFrom.y -= 0.5f * myDeltaTime;

			myCamera.Setup(
				myRenderingParameters.myLookFrom,
				myRenderingParameters.myLookAt,
				myRenderingParameters.myUp,
				myRenderingParameters.myVFov,
				myRenderingParameters.myAspectRatio,
				myRenderingParameters.myAperature,
				myRenderingParameters.myDistToFocus);

			//myWorld.ClearOctree();
			//myWorld.BuildOctree();

			FW_GrowingArray<Sphere>& spheres = myWorld.GetAllSpheres();
			for (Sphere& sphere : spheres)
			{
				if (sphere.myMaterial.myMaterialType == Material::MaterialType::LightSource)
				{
					sphere.myMaterial.myInterpolator.Tick(myDeltaTime);
				}
			}

			myImageRenderer = new ImageRenderer(myCamera, myWorld, myRenderingParameters, myTexture.mySize.x, myTexture.mySize.y);
		}
	}
}

void Raytracer::PrepareTextureForRender()
{
	const int imageWidth = myRenderingParameters.myImageWidth;
	const int imageHeight = static_cast<int>(imageWidth / myRenderingParameters.myAspectRatio);
	FW_Renderer::ResizeOffscreenBuffer(imageWidth, myRenderingParameters.myAspectRatio);

	FW_Renderer::DeleteTexture(myTexture);
	myTexture = FW_Renderer::CreateTexture({ imageWidth, imageHeight });

	myTexturePixels.RemoveAll();
	myTexturePixels.Reserve(myTexture.mySize.x * myTexture.mySize.y);

	for (int y = 0; y < myTexture.mySize.y; ++y)
	{
		for (int x = 0; x < myTexture.mySize.x; ++x)
		{
			int pixelIndex = y * myTexture.mySize.x + x;
			myTexturePixels[pixelIndex] = 0xFFFFFFFF;
		}
	}
	FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());
}

void Raytracer::TryUpdateRenderTexture()
{
	FW_Time::TimeUnit currentTime = FW_Time::GetTime();
	float duration = FW_Time::ConvertTimeUnitToGameTime(currentTime - myLastRenderTextureUpdateTime);

	if (duration >= 1.f)
	{
		myImageRenderer->CopyCurrentImageState(myTexturePixels);
		myLastRenderTextureUpdateTime = FW_Time::GetTime();
		FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());
	}
}

bool Raytracer::TryToFinalizeTexture()
{
	if (myImageRenderer->IsFinished())
	{
		myImageRenderer->CopyCurrentImageState(myTexturePixels);
		FW_Renderer::UpdatePixelsInTexture(myTexture, myTexturePixels.GetArrayAsPointer());

		FW_Time::TimeUnit endTime = FW_Time::GetTime();
		float duration = FW_Time::ConvertTimeUnitToGameTime(endTime - myImageRenderer->GetStartTime());

		FW_String message;
		if (myCurrentState == RENDERING_IMAGE)
		{
			message = "Image took ";
		}
		else if (myCurrentState == RENDERING_VIDEO)
		{
			message = "Frame ";
			message += myVideoFrameToRender + 1;
			message += " took ";
		}

		message += duration;
		message += "s to render";
		FW_Logger::AddMessage(message.GetBuffer());

		delete myImageRenderer;
		myImageRenderer = nullptr;

		return true;
	}

	return false;
}

void Raytracer::SaveSceneToDisk()
{
	FW_FileProcessor file("Raytracer/Data/Scene.txt", FW_FileProcessor::Flags::WRITE);

	file.Process(myRenderingParameters.myBackgroundColor);
	file.Process(myRenderingParameters.myUseFlatBackground);

	// Camera Parameters
	file.Process(myRenderingParameters.myLookFrom);
	file.Process(myRenderingParameters.myLookAt);
	file.Process(myRenderingParameters.myUp);
	file.Process(myRenderingParameters.myAperature);
	file.Process(myRenderingParameters.myDistToFocus);
	file.Process(myRenderingParameters.myVFov);
	file.Process(myRenderingParameters.myAspectRatio);

	// Video Parameters
	file.Process(myRenderingParameters.myVideoLenght);
	file.Process(myRenderingParameters.myFPS);


	FW_GrowingArray<Sphere>& spheres = myWorld.GetAllSpheres();
	int numSpheres = spheres.Count();
	file.Process(numSpheres);

	for (Sphere& sphere : spheres)
	{
		file.Process(sphere.myMaterial);
		file.Process(sphere.myUIName);
		file.Process(sphere.myPosition);
		file.Process(sphere.myRadius);
	}
}

void Raytracer::LoadSceneFromDisk()
{
	myWorld.ClearWorld();

	FW_FileProcessor file("Raytracer/Data/Scene.txt", FW_FileProcessor::Flags::READ);

	file.Process(myRenderingParameters.myBackgroundColor);
	file.Process(myRenderingParameters.myUseFlatBackground);

	// Camera Parameters
	file.Process(myRenderingParameters.myLookFrom);
	file.Process(myRenderingParameters.myLookAt);
	file.Process(myRenderingParameters.myUp);
	file.Process(myRenderingParameters.myAperature);
	file.Process(myRenderingParameters.myDistToFocus);
	file.Process(myRenderingParameters.myVFov);
	file.Process(myRenderingParameters.myAspectRatio);

	// Video Parameters
	file.Process(myRenderingParameters.myVideoLenght);
	file.Process(myRenderingParameters.myFPS);

	int numSpheres = 0;
	file.Process(numSpheres);
	for (int i = 0; i < numSpheres; ++i)
	{
		Sphere sphere;

		file.Process(sphere.myMaterial);
		file.Process(sphere.myUIName);
		file.Process(sphere.myPosition);
		file.Process(sphere.myRadius);

		myWorld.AddObject(sphere);
	}
}
