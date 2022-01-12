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

		// Figure out a way to not clear input when the ImGUI-Widget that contains the game-texture is focused?
		//ImGuiIO& io = ImGui::GetIO();
		//if (io.WantCaptureKeyboard || io.WantCaptureMouse)
		//	SFML_Input::ClearInput();

		if (FW_Input::WasKeyReleased(FW_Input::KeyCode::ESC))
			myRenderWindow->close();

		ImGui::SFML::Update(*myRenderWindow, FW_Time::GetDelta());

		FW_Renderer::Clear();

		aGame.Run();

		FW_Renderer::FinishOffscreenBuffer();

		BuildImGUIStuff(aGame);

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

void SFML_Engine::BuildImGUIStuff(FW_IGame& aGame)
{
	FW_String consoleTitle = "Wow amazing (";
	consoleTitle += int(FW_Time::GetAverageFramerate());
	consoleTitle += " fps)###DummyID";

	ImVec2 imguiSize = myRenderWindow->getSize();
	imguiSize.x -= 20;
	imguiSize.y -= 20;
	ImGui::SetNextWindowSize(imguiSize);
	ImGui::Begin(consoleTitle.GetBuffer());

	ImGui::BeginGroup();
	
	ImTextureID textureID = (ImTextureID)SFML_Renderer::GetOffscreenBuffer().getNativeHandle();
	aGame.BuildGameImguiEditor(textureID);

	{
		// Console/log stuff

		ImGui::Text("Console & Log");
		ImGui::BeginGroup();
		ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)2), ImVec2(0, -30), true);

		const FW_CircularArray<FW_String, 128>& logMessage = FW_Logger::GetAllMessages();
		for(int i = 0; i < logMessage.myTotalCount; ++i)
			ImGui::Text(logMessage[i].GetBuffer());

		//for (int item = 0; item < 100; item++)
		//	ImGui::Text("Item %d", item);

		ImGui::EndChild();

		static char buf1[64] = ""; ImGui::InputText("Input", buf1, 64);
		ImGui::EndGroup();
	}

	ImGui::EndGroup();
	ImGui::End();
}
