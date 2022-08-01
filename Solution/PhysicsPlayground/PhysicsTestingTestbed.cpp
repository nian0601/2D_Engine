#include "PhysicsTestingTestbed.h"
#include <FW_Renderer.h>
#include <FW_Input.h>
#include "ChainBuilder.h"
#include <FW_Time.h>
#include <FW_Math.h>
#include <imgui\imgui.h>

PhysicsTestingTestbed::PhysicsTestingTestbed()
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

PhysicsTestingTestbed::~PhysicsTestingTestbed()
{

}

void PhysicsTestingTestbed::Run()
{
	const Vector2f& mousePos = FW_Input::GetMousePositionf();

	if (FW_Input::WasMouseReleased(FW_Input::RIGHTMB))
		myPhysicsWorld.ApplyForceInRadius(mousePos, 200.f, 0.f, 100000.f);

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
		playerForce.x -= 100.f;
	if (FW_Input::IsKeyDown(FW_Input::D))
		playerForce.x += 100.f;
	if (FW_Input::WasKeyReleased(FW_Input::SPACE))
		playerForce.y -= 50000.f;

	if (myPlayerObject)
		myPlayerObject->myVelocity += myPlayerObject->myInvMass * playerForce;

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

	for (const PhysicsObject* object : myPhysicsWorld.GetObjects())
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
}


void PhysicsTestingTestbed::GenerateScene(SceneType aSceneType)
{
	myPhysicsWorld.DeleteAllObjects();

	switch (aSceneType)
	{
	case PhysicsTestingTestbed::RandomCircles:
		GenerateRandomCirclesScene();
		break;
	case PhysicsTestingTestbed::ChainTest:
		GenerateChainTestScene();
		break;
	case PhysicsTestingTestbed::BoxesAndCircles:
		GenerateBoxesAndCirclesScene();
		break;
	case PhysicsTestingTestbed::Polygon:
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
	PhysicsObject* topEdge = new PhysicsObject(new AABBShape(rect.myExtents));
	topEdge->SetPosition(rect.myCenterPos);
	topEdge->MakeStatic();
	topEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(topEdge);

	rect = MakeRect(x1, y2 - thickness, x2, y2);
	PhysicsObject* bottomEdge = new PhysicsObject(new AABBShape(rect.myExtents));
	bottomEdge->SetPosition(rect.myCenterPos);
	bottomEdge->MakeStatic();
	bottomEdge->myColor = 0xFF444444;
	bottomEdge->myRestitution = 0.1f;
	myPhysicsWorld.AddObject(bottomEdge);

	rect = MakeRect(x1, y1, x1 + thickness, y2);
	PhysicsObject* leftEdge = new PhysicsObject(new AABBShape(rect.myExtents));
	leftEdge->SetPosition(rect.myCenterPos);
	leftEdge->MakeStatic();
	leftEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(leftEdge);

	rect = MakeRect(x2 - thickness, y1, x2, y2);
	PhysicsObject* rightEdge = new PhysicsObject(new AABBShape(rect.myExtents));
	rightEdge->SetPosition(rect.myCenterPos);
	rightEdge->MakeStatic();
	rightEdge->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(rightEdge);

	myPlayerObject = new PhysicsObject(new CircleShape(10.f));
	myPlayerObject->SetPosition({ 500.f, 300.f });
	myPlayerObject->myRestitution = 0.2f;
	myPhysicsWorld.AddObject(myPlayerObject);
#endif
}

void PhysicsTestingTestbed::GenerateRandomCirclesScene()
{
	for (int i = 0; i < myNumberOfCircles; ++i)
	{
		float radius = static_cast<float>(FW_RandInt(myMinMaxRadius.x, myMinMaxRadius.y));
		PhysicsObject* circle = new PhysicsObject(new CircleShape(radius));

		circle->myRestitution = FW_RandFloat(myMinMaxRestitution.x, myMinMaxRestitution.y);
		circle->SetPosition({ FW_RandFloat(50.f, 600.f) , FW_RandFloat(50.f, 300.f) });
		circle->myVelocity = FW_RandomVector2f() * FW_RandFloat(50.f, 150.f);
		circle->myColor = FW_RandomColor();

		myPhysicsWorld.AddObject(circle);
	}
}

void PhysicsTestingTestbed::GenerateChainTestScene()
{
	float radius = 10.f;
	PhysicsObject* anchor = new PhysicsObject(new CircleShape(radius));
	anchor->SetMass(0);
	anchor->SetPosition({ 200.f, 200.f });
	anchor->myColor = 0xFF444444;
	myPhysicsWorld.AddObject(anchor);

	PhysicsObject* prevLink = anchor;
	int chainLenght = 5;
	for (int i = 0; i < chainLenght; ++i)
	{
		PhysicsObject* newLink = new PhysicsObject(new CircleShape(radius));
		newLink->SetMass(radius);
		newLink->myRestitution = 0.7f;

		newLink->SetPosition(prevLink->myPosition + radius * 2);
		myPhysicsWorld.AddObject(newLink);

		MaxDistanceConstraint* constraint = new MaxDistanceConstraint();
		constraint->myObjectA = prevLink;
		constraint->myObjectB = newLink;
		constraint->myMaxDistance = 20.f;
		myPhysicsWorld.AddConstraint(constraint);

		prevLink = newLink;
	}

	PhysicsObject* endAnchor = new PhysicsObject(new CircleShape(radius));
	endAnchor->SetMass(0);
	endAnchor->myColor = 0xFF444444;
	endAnchor->SetPosition(prevLink->myPosition + radius * 2);
	myPhysicsWorld.AddObject(endAnchor);

	MaxDistanceConstraint* constraint = new MaxDistanceConstraint();
	constraint->myObjectA = prevLink;
	constraint->myObjectB = endAnchor;
	constraint->myMaxDistance = 20.f;
	myPhysicsWorld.AddConstraint(constraint);
}

void PhysicsTestingTestbed::GenerateBoxesAndCirclesScene()
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

		PhysicsObject* box = new PhysicsObject(new AABBShape(extents));
		box->myPosition = position;
		box->myColor = FW_RandomColor();
		box->SetMass(1.f);
		box->myRestitution = 0.7f;
		myPhysicsWorld.AddObject(box);

		position.y -= extents.y * 0.5f;
	}
}

void PhysicsTestingTestbed::GeneratePolygonScene()
{
	Vector2f extents;
	extents.x = 40.f;
	extents.y = 60.f;

	Vector2f position;
	position.x = 200.f;
	position.y = 300.f;

	PhysicsObject* polygon = new PhysicsObject(new AABBShape(extents));
	polygon->SetPosition(position);
	//polygon->SetOrientation(FW_DegreesToRadians(-40.f));
	polygon->myColor = FW_RandomColor();
	polygon->myRestitution = 0.2f;
	polygon->myDynamicFriction = 0.2f;
	polygon->myStaticFriction = 0.4f;
	polygon->MakeStatic();
	//polygon->SetInertia(0.f);


	myPhysicsWorld.AddObject(polygon);

	extents.x = 200.f;
	extents.y = 40.f;

	position.x = 200.f;
	position.y = 200.f;

	polygon = new PhysicsObject(new AABBShape(extents));
	polygon->SetPosition(position);
	polygon->SetOrientation(FW_DegreesToRadians(-40.f));
	polygon->myColor = FW_RandomColor();
	polygon->myRestitution = 0.2f;
	polygon->myDynamicFriction = 0.2f;
	polygon->myStaticFriction = 0.4f;
	polygon->MakeStatic();
	//polygon->SetInertia(0.f);

	myPhysicsWorld.AddObject(polygon);
}
