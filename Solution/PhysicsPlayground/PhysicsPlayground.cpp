#include "PhysicsPlayground.h"
#include "FW_Includes.h"
#include <FW_Math.h>
#include <FW_Logger.h>
#include "FW_FileProcessor.h"
#include <FW_Time.h>
#include "ChainBuilder.h"

int RandomColor()
{
	int color = 0;

	Vector3f randColor = FW_RandomVector3f(0.f, 1.f);

	color |= unsigned char(FW_Clamp(randColor.x, 0.f, 0.999f) * 255.99f) << 0;
	color |= unsigned char(FW_Clamp(randColor.y, 0.f, 0.999f) * 255.99f) << 8;
	color |= unsigned char(FW_Clamp(randColor.z, 0.f, 0.999f) * 255.99f) << 16;
	color |= 255 << 24;

	return color;
}

void PhysicsPlayground::OnStartup()
{
	const float aspectRatio = 16.f / 9.f;
	const float imageWidth = 720.f;

	FW_Renderer::ResizeOffscreenBuffer(720, aspectRatio);

	myIsInSingleFrameMode = true;
	myShouldStepSingleFrame = false;
	myChainBuilder = nullptr;

	myNumberOfCircles = 15;
	myMinMaxRadius.x = 5;
	myMinMaxRadius.y = 15;
	myMinMaxRestitution.x = 0.6f;
	myMinMaxRestitution.y = 0.9f;

	GenerateNewScene();

	
}

void PhysicsPlayground::OnShutdown()
{
}

bool PhysicsPlayground::Run()
{
	Vector2f mousePos = FW_Input::GetMousePositionf();
	// Magic number-adjustments to move the MousePos to the topleft corner of the game-window
	// Surely there is some better way of doing this using ImGui-stuff?
	mousePos.x -= 366.f;
	mousePos.y -= 27.f;

	if (FW_Input::WasMouseReleased(FW_Input::LEFTMB))
		myPhysicsWorld.ApplyForceInRadius(mousePos, 200.f, 0.f, 500000.f);

	if (myChainBuilder)
	{
		if (!myChainBuilder->Update(mousePos))
		{
			delete myChainBuilder;
			myChainBuilder = nullptr;
		}
	}

	Vector2f playerForce;
	if (FW_Input::IsKeyDown(FW_Input::A))
		playerForce.x -= 200.f;
	if (FW_Input::IsKeyDown(FW_Input::D))
		playerForce.x += 200.f;
	if (FW_Input::WasKeyReleased(FW_Input::SPACE))
		playerForce.y -= 100000.f;

	myPlayerObject->myForces += playerForce;

	static float accumulator = 0.f;

	float delta = FW_Time::GetDelta();

	accumulator += delta;
	accumulator = FW_Clamp(accumulator, 0.f, 1.f);

	while (accumulator >= myPhysicsWorld.GetFixedDeltaTime())
	{
		if (myIsInSingleFrameMode)
		{
			if (myShouldStepSingleFrame)
			{
				myPhysicsWorld.Tick();
				myShouldStepSingleFrame = false;
			}
		}
		else
		{
			myPhysicsWorld.Tick();
		}
		
		accumulator -= myPhysicsWorld.GetFixedDeltaTime();
	}

	

	for (const Object* object : myPhysicsWorld.GetObjects())
		object->myShape->Render();

	return true;
}

void PhysicsPlayground::BuildGameImguiEditor(unsigned int aGameOffscreenBufferTextureID)
{
	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)55), ImVec2(350, 500), false);

	ImGui::Checkbox("Single Frame Mode", &myIsInSingleFrameMode);
	ImGui::SameLine();
	if (ImGui::Button("Step Single Frame"))
		myShouldStepSingleFrame = true;

	ImGui::DragFloat("Gravity Scale", &myPhysicsWorld.myGravityScale, 0.1f, -20.f, 20.f);

	

	ImGui::Separator();

	ImGui::DragInt("Number of Circles", &myNumberOfCircles, 1, 100);
	ImGui::DragInt2("Min/Max Radius", &myMinMaxRadius.x, 1, 1, 100);
	ImGui::DragFloat2("Min/Max Restitution", &myMinMaxRestitution.x, 0.01f, 0.f, 1.f);

	if (ImGui::Button("Generate New Scene"))
	{
		myIsInSingleFrameMode = true;
		GenerateNewScene();
	}

	if (!myChainBuilder)
	{
		if (ImGui::Button("Create Chain"))
			myChainBuilder = new ChainBuilder(myPhysicsWorld);
	}
	else
	{
		if (ImGui::Button("Cancle Chain"))
		{
			delete myChainBuilder;
			myChainBuilder = nullptr;
		}
	}
	

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)56), ImVec2(0, 500), false);

	const float imageWidth = static_cast<float>(FW_Renderer::GetOffscreenBufferWidth());
	const float imageHeight = static_cast<float>(FW_Renderer::GetOffscreenBufferHeight());
	ImGui::Image(aGameOffscreenBufferTextureID, ImVec2(imageWidth, imageHeight), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::EndChild();
}

void PhysicsPlayground::GenerateNewScene()
{
	myPhysicsWorld.DeleteAllObjects();

#if 0
	for (int i = 0; i < myNumberOfCircles; ++i)
	{
		float radius = static_cast<float>(FW_RandInt(myMinMaxRadius.x, myMinMaxRadius.y));
		Object* circle = new Object(new CircleShape(radius));
		circle->SetMass(radius);

		circle->myRestitution = FW_RandFloat(myMinMaxRestitution.x, myMinMaxRestitution.y);
		circle->myPosition.x = FW_RandFloat(50.f, 600.f);
		circle->myPosition.y = FW_RandFloat(50.f, 300.f);
		circle->myVelocity = FW_RandomVector2f() * FW_RandFloat(50.f, 150.f);
		circle->myColor = RandomColor();

		myPhysicsWorld.AddObject(circle);
	}
#else
	float radius = 10.f;
	Object* anchor = new Object(new CircleShape(radius));
	anchor->SetMass(0);
	anchor->myPosition = { 200.f, 200.f };
	anchor->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(anchor);

	Object* prevLink = anchor;
	int chainLenght = 5;
	for (int i = 0; i < chainLenght; ++i)
	{
		Object* newLink = new Object(new CircleShape(radius));
		newLink->SetMass(radius);
		newLink->myRestitution = 0.7f;

		newLink->myPosition = prevLink->myPosition;
		newLink->myPosition.y += radius * 2;
		myPhysicsWorld.AddObject(newLink);

		MaxDistanceConstraint* constraint = new MaxDistanceConstraint();
		constraint->myObjectA = prevLink;
		constraint->myObjectB = newLink;
		constraint->myMaxDistance = 20.f;
		myPhysicsWorld.AddConstraint(constraint);

		prevLink = newLink;
	}

	Object* endAnchor = new Object(new CircleShape(radius));
	endAnchor->SetMass(0);
	endAnchor->myColor = 0xFF444444;
	endAnchor->myPosition = prevLink->myPosition;
	endAnchor->myPosition.y += radius * 2;
	myPhysicsWorld.AddObject(endAnchor);

	MaxDistanceConstraint* constraint = new MaxDistanceConstraint();
	constraint->myObjectA = prevLink;
	constraint->myObjectB = endAnchor;
	constraint->myMaxDistance = 20.f;
	myPhysicsWorld.AddConstraint(constraint);
#endif

	const int width = FW_Renderer::GetOffscreenBufferWidth();
	const int height = FW_Renderer::GetOffscreenBufferHeight();

	const float x1 = 0.f;
	const float x2 = static_cast<float>(width);
	const float y1 = 0.f;
	const float y2 = static_cast<float>(height);

	const float thickness = 10.f;

	Rectf rect = MakeRect(x1, y1, x2, y1 + thickness);
	Object* topEdge = new Object(new AABBShape(rect));
	topEdge->myPosition = rect.myCenterPos;
	topEdge->SetMass(0);
	topEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(topEdge);

	rect = MakeRect(x1, y2 - thickness, x2, y2);
	Object* bottomEdge = new Object(new AABBShape(rect));
	bottomEdge->myPosition = rect.myCenterPos;
	bottomEdge->SetMass(0);
	bottomEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(bottomEdge);

	rect = MakeRect(x1, y1, x1 + thickness, y2);
	Object* leftEdge = new Object(new AABBShape(rect));
	leftEdge->myPosition = rect.myCenterPos;
	leftEdge->SetMass(0);
	leftEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(leftEdge);

	rect = MakeRect(x2 - thickness, y1, x2, y2);
	Object* rightEdge = new Object(new AABBShape(rect));
	rightEdge->myPosition = rect.myCenterPos;
	rightEdge->SetMass(0);
	rightEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(rightEdge);

	rect = MakeRect(x1 + 300, y1 + 200, x1 + 500, y1 + 220);
	Object* center = new Object(new AABBShape(rect));
	center->myPosition = rect.myCenterPos;
	center->SetMass(0);
	center->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(center);

	myPlayerObject = new Object(new CircleShape(10.f));
	myPlayerObject->myPosition = { 500.f, 300.f };
	myPlayerObject->SetMass(25.f);
	myPlayerObject->myRestitution = 0.2f;
	myPhysicsWorld.AddObject(myPlayerObject);
}
