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

	mySelectedSceneType = SceneType::Polygon;
	GenerateScene(static_cast<SceneType>(mySelectedSceneType));

	myRenderContacts = false;
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

	if (FW_Input::WasMouseReleased(FW_Input::RIGHTMB))
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

	if (myPlayerObject)
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

	if (myRenderContacts)
	{
		for (const Manifold& contact : myPhysicsWorld.GetContacts())
		{
			for (int i = 0; i < contact.myContactCount; ++i)
			{
				FW_Renderer::RenderLine(contact.myContacts[i], contact.myContacts[i] + contact.myHitNormal * contact.myPenetrationDepth, 0xFFFF0000);
				FW_Renderer::RenderCircle(contact.myContacts[i], 3.f);
			}
		}
	}

	ImGui::SetNextWindowSize({ 400.f, static_cast<float>(FW_Renderer::GetScreenHeight() - 10.f) });
	ImGui::SetNextWindowPos({ static_cast<float>(FW_Renderer::GetScreenWidth()) - 405.f, 5.f });
	ImGui::Begin("PhysicsPlayground", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	ImGui::Checkbox("Single Frame Mode", &myIsInSingleFrameMode);
	ImGui::SameLine();
	if (ImGui::Button("Step Single Frame"))
		myShouldStepSingleFrame = true;

	ImGui::DragFloat("Gravity Scale", &PhysicsWorld::ourGravityScale, 0.1f, -20.f, 20.f);
	ImGui::Checkbox("Render Contacts", &myRenderContacts);


	ImGui::Separator();

	ImGui::DragInt("Number of Circles", &myNumberOfCircles, 1, 100);
	ImGui::DragInt2("Radius", &myMinMaxRadius.x, 1, 1, 100);
	ImGui::DragFloat2("Restitution", &myMinMaxRestitution.x, 0.01f, 0.f, 1.f);

	if (ImGui::Button("Generate"))
	{
		myIsInSingleFrameMode = true;
		GenerateScene(static_cast<SceneType>(mySelectedSceneType));
	}

	ImGui::SameLine();
	const char* items[] = { "RandomCircles", "ChainTest", "BoxesAndCircles" };
	ImGui::Combo("Scene", &mySelectedSceneType, items, IM_ARRAYSIZE(items));

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

	ImGui::End();

	return true;
}

void PhysicsPlayground::GenerateScene(SceneType aSceneType)
{
	myPhysicsWorld.DeleteAllObjects();

	switch (aSceneType)
	{
	case PhysicsPlayground::RandomCircles:
		GenerateRandomCirclesScene();
		break;
	case PhysicsPlayground::ChainTest:
		GenerateChainTestScene();
		break;
	case PhysicsPlayground::BoxesAndCircles:
		GenerateBoxesAndCirclesScene();
		break;
	case PhysicsPlayground::Polygon:
		GeneratePolygonScene();
		break;
	default:
		break;
	}

#if 1
	const int width = FW_Renderer::GetOffscreenBufferWidth();
	const int height = FW_Renderer::GetOffscreenBufferHeight();

	const float x1 = 0.f;
	const float x2 = static_cast<float>(width);
	const float y1 = 0.f;
	const float y2 = static_cast<float>(height);

	const float thickness = 30.f;

	Rectf rect = MakeRect(x1, y1, x2, y1 + thickness);
	Object* topEdge = new Object(new PolygonShape(rect.myExtents));
	topEdge->myPosition = rect.myCenterPos;
	topEdge->SetStatic();
	topEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(topEdge);

	rect = MakeRect(x1, y2 - thickness*2, x2, y2-thickness);
	Object* bottomEdge = new Object(new PolygonShape(rect.myExtents));
	bottomEdge->myPosition = rect.myCenterPos;
	bottomEdge->SetStatic();
	bottomEdge->myColor = 0xFF444444;
	bottomEdge->myRestitution = 0.1f;
	myPhysicsWorld.AddObject(bottomEdge);

	rect = MakeRect(x1, y1, x1 + thickness, y2);
	Object* leftEdge = new Object(new PolygonShape(rect.myExtents));
	leftEdge->myPosition = rect.myCenterPos;
	leftEdge->SetStatic();
	leftEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(leftEdge);

	rect = MakeRect(x2 - thickness, y1, x2, y2);
	Object* rightEdge = new Object(new PolygonShape(rect.myExtents));
	rightEdge->myPosition = rect.myCenterPos;
	rightEdge->SetStatic();
	rightEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(rightEdge);

	myPlayerObject = new Object(new CircleShape(10.f));
	myPlayerObject->myPosition = { 500.f, 300.f };
	myPlayerObject->myRestitution = 0.2f;
	myPhysicsWorld.AddObject(myPlayerObject);
#endif
}

void PhysicsPlayground::GenerateRandomCirclesScene()
{
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
}

void PhysicsPlayground::GenerateChainTestScene()
{
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
}

void PhysicsPlayground::GenerateBoxesAndCirclesScene()
{
	const int width = FW_Renderer::GetOffscreenBufferWidth();
	const int height = FW_Renderer::GetOffscreenBufferHeight();

	const Vector2i widthSpan = { 50, 80 };
	const Vector2i heightSpan = { 50, 80 };

	Vector2f position = { 300.f, height - 30.f };

	for (int i = 0; i < 5; ++i)
	{
		Vector2f extents;
		extents.x = static_cast<float>(FW_RandInt(widthSpan.x, widthSpan.y));
		extents.y = static_cast<float>(FW_RandInt(heightSpan.x, heightSpan.y));
		position.y -= 10.f;
		position.y -= extents.y * 0.5f;

		Rectf rect = MakeRect(position, extents);
		Object* box = new Object(new AABBShape(rect));
		box->myPosition = rect.myCenterPos;
		box->myColor = RandomColor();
		box->SetMass(1.f);
		box->myRestitution = 0.7f;
		myPhysicsWorld.AddObject(box);

		
		position.y -= extents.y * 0.5f;
	}
}

void PhysicsPlayground::GeneratePolygonScene()
{
	Vector2f extents;
	extents.x = 40.f;
	extents.y = 60.f;

	Vector2f position;
	position.x = 200.f;
	position.y = 200.f;

	Object* polygon = new Object(new PolygonShape(extents));
	polygon->myPosition = position;
	polygon->myColor = RandomColor();
	polygon->SetOrientation(FW_DegreesToRadians(-40.f));
	polygon->myRestitution = 0.2f;
	polygon->myDynamicFriction = 0.2f;
	polygon->myStaticFriction = 0.4f;
	//polygon->SetInertia(0.f);
	

	myPhysicsWorld.AddObject(polygon);

	extents.x = 200.f;
	extents.y = 40.f;

	position.x = 200.f;
	position.y = 50.f;

	polygon = new Object(new PolygonShape(extents));
	polygon->myPosition = position;
	polygon->myColor = RandomColor();
	//polygon->SetOrientation(FW_DegreesToRadians(-40.f));
	polygon->myRestitution = 0.2f;
	polygon->myDynamicFriction = 0.2f;
	polygon->myStaticFriction = 0.4f;
	//polygon->SetInertia(0.f);

	myPhysicsWorld.AddObject(polygon);
}
