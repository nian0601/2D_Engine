#include "SFML_Engine.h"

#include "FW_Input.h"
#include "SFML_Input.h"
#include "FW_Renderer.h"
#include "SFML_Renderer.h"

#include "FW_IGame.h"
#include "FW_Time.h"

#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

SFML_Engine::SFML_Engine(int aWidth, int aHeight, const char* aWindowTitle)
{
	myRenderWindow = new sf::RenderWindow(sf::VideoMode(aWidth, aHeight), aWindowTitle);
	SFML_Renderer::Init(myRenderWindow);
	ImGui::SFML::Init(*myRenderWindow);
	FW_Time::Init();
}

void SFML_Engine::Run(FW_IGame& aGame)
{
	char windowTitle[255] = "ImGui + SFML = <3";

	while (myRenderWindow->isOpen())
	{
		sf::Event event;
		while (myRenderWindow->pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
				myRenderWindow->close();

			if (event.type == sf::Event::GainedFocus)
				SFML_Input::SetWindowHasFocus(true);

			if (event.type == sf::Event::LostFocus)
				SFML_Input::SetWindowHasFocus(false);
		}

		SFML_Input::FlushInput(*myRenderWindow);
		FW_Time::Update();

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
			SFML_Input::ClearInput();

		if (FW_Input::WasKeyReleased(FW_Input::KeyCode::ESC))
			myRenderWindow->close();

		ImGui::SFML::Update(*myRenderWindow, FW_Time::GetDelta());

		FW_Renderer::Clear();

		aGame.Run();

		FW_Renderer::RenderFloat(1.f / FW_Time::GetDelta(), { 5, 5 });

		ImGui::SFML::Render(*myRenderWindow);

		FW_Renderer::Present();
	}

	SFML_Renderer::Shutdown();
	delete myRenderWindow;
}
