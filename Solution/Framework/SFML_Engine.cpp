#include "SFML_Engine.h"

#include "FW_Input.h"
#include "SFML_Input.h"
#include "FW_Renderer.h"
#include "SFML_Renderer.h"
#include "SFML_AudioSystem.h"

#include "FW_IGame.h"
#include "FW_Time.h"

#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "FW_String.h"
#include "FW_Logger.h"
#include "FW_LevelLoader.h"

SFML_Engine::SFML_Engine(int aWidth, int aHeight, const char* aWindowTitle)
{
	myRenderWindow = new sf::RenderWindow(sf::VideoMode(aWidth, aHeight), aWindowTitle);
	SFML_Renderer::Init(myRenderWindow);
	ImGui::SFML::Init(*myRenderWindow);
	FW_Time::Init();
	SFML_AudioSystem::Init();
	
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
}

void SFML_Engine::Run(FW_IGame& aGame)
{
	myRenderWindow->setTitle(aGame.GetGameName());
	SFML_Renderer::SetDataFolder(aGame.GetDataFolderName());
	SFML_AudioSystem::SetDataFolder(aGame.GetDataFolderName());
	FW_LevelLoader::SetDataFolder(aGame.GetDataFolderName());

	aGame.OnStartup();

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
		if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_F1))
			myShowConsole = !myShowConsole;
		if (FW_Input::WasKeyReleased(FW_Input::KeyCode::_F2))
			myShowIMGuiDemoWindow = !myShowIMGuiDemoWindow;

		ImGui::SFML::Update(*myRenderWindow, FW_Time::GetDelta());

		FW_Renderer::Clear();

		aGame.Run();

		FW_Renderer::FinishOffscreenBuffer();

		BuildImGUIStuff();

		ImGui::SFML::Render(*myRenderWindow);

		FW_Renderer::Present();
	}

	SFML_Renderer::Shutdown();
	SFML_AudioSystem::Shutdown();
	aGame.OnShutdown();
	delete myRenderWindow;
}

void SFML_Engine::SetWindowPosition(int aX, int aY)
{
	myRenderWindow->setPosition({ aX, aY });
}

void SFML_Engine::BuildImGUIStuff()
{
	if (myShowConsole)
	{
		FW_String consoleTitle = "Console (";
		consoleTitle += int(FW_Time::GetAverageFramerate());
		consoleTitle += " fps)###DummyID";

		ImGui::SetNextWindowSize({ static_cast<float>(FW_Renderer::GetScreenWidth()), 150.f });
		ImGui::Begin(consoleTitle.GetBuffer(), nullptr, ImGuiWindowFlags_NoDecoration);

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)2), ImVec2(0, -30), true);

		const FW_CircularArray<FW_String, 128>& logMessage = FW_Logger::GetAllMessages();
		for (int i = 0; i < logMessage.myTotalCount; ++i)
			ImGui::Text(logMessage[i].GetBuffer());

		ImGui::EndChild();

		if (ImGui::InputText("Input", myConsoleInputBuffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			FW_Logger::AddMessage(myConsoleInputBuffer);
			myConsoleInputBuffer[0] = '\0';
		}

		ImGui::End();
	}

	if(myShowIMGuiDemoWindow)
		ImGui::ShowDemoWindow();
}
