#include "FW_Editor.h"
#include "imgui\imgui.h"
#include "FW_EntityManager.h"
#include "FW_FileSystem.h"
#include "FW_Components.h"
#include "FW_LevelLoader.h"
#include "FW_CollisionSystem.h"

namespace FW_Editor
{
	static bool ourEditorIsEnabled = false;
	static bool ourShowDemoWindow = false;

	static FW_EntityManager* ourEntityStorage = nullptr;

	static FW_GrowingArray<Vector2f> ourNewEntityPositions;
	static FW_GrowingArray<FW_EntityID> ourSelectedEntities;

	void HandleMenuAction(const std::string& anAction, FW_EntityManager& aEntityStorage)
	{
		if (anAction == "New")
		{
			aEntityStorage.QueueRemovalAllEntities();
		}
		else if (anAction == "SaveCurrent")
		{
			FW_LevelLoader::SaveLevel(aEntityStorage, "testlevel");
		}
		else if (anAction == "SaveAs")
		{
			ImGui::OpenPopup("Save Level As");
		}

		if (ImGui::BeginPopupModal("Save Level As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetItemDefaultFocus();
			static char filename[64];
			ImGui::InputText("Filename", filename, sizeof(filename));

			if (ImGui::Button("Save", ImVec2(120, 0)))
			{
				if (strlen(filename) > 0)
				{
					FW_LevelLoader::SaveLevel(aEntityStorage, filename);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}

	bool BeginEditor(FW_EntityManager& aEntityStorage)
	{
		std::string menuAction;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New")) menuAction = "New";
				if (ImGui::MenuItem("Save As testlevel")) menuAction = "SaveCurrent";
				if (ImGui::MenuItem("Save As ...")) menuAction = "SaveAs";

				if (ImGui::BeginMenu("Open"))
				{
					FW_GrowingArray<FW_FileSystem::FileInfo> levelFiles;
					FW_FileSystem::GetAllFilesFromDirectory(FW_LevelLoader::ourLevelPath, levelFiles);

					ImGui::Text("Levels");
					ImGui::Separator();

					for (const FW_FileSystem::FileInfo& levelFile : levelFiles)
					{
						if (ImGui::MenuItem(levelFile.myFileNameNoExtention.GetBuffer()))
						{
							aEntityStorage.QueueRemovalAllEntities();
							aEntityStorage.FlushEntityRemovals();
							FW_LevelLoader::LoadLevel(aEntityStorage, levelFile.myFileNameNoExtention.GetBuffer());
							break;
						}
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		HandleMenuAction(menuAction, aEntityStorage);

		ImGui::Begin("Level Editor");
		ImGui::Checkbox("Show Demo Window", &ourShowDemoWindow);
		ImGui::Checkbox("Edit", &ourEditorIsEnabled);
		ImGui::Separator();

		if(ourShowDemoWindow)
			ImGui::ShowDemoWindow();

		FW_ASSERT(ourEntityStorage == nullptr, "BeginEditor was called without then calling EndEditor");

		if (ourEditorIsEnabled)
			ourEntityStorage = &aEntityStorage;
		else
			ImGui::End();

		return ourEditorIsEnabled;
	}

	void NewEntityPopup(const FW_GrowingArray<Vector2f>& somePositions)
	{
		if (ourSelectedEntities.IsEmpty())
		{
			ourNewEntityPositions = somePositions;
			ImGui::OpenPopup("new_entity_popup");
		}
	}

	void EditEntity(FW_EntityID anEntity)
	{
		ImGui::Text("ENTITY:");
		ourEntityStorage->BuildComponentUI(anEntity);

		ImGui::Spacing();

		if (ImGui::Button("Save"))
		{
			FileResoureComponent* resourceComponent = ourEntityStorage->FindComponent<FileResoureComponent>(anEntity);
			FW_ASSERT(resourceComponent != nullptr, "Nullptr ResourceComponent when saving entity, HOW?!");

			FW_String filePath = "Data/Entities/";
			filePath += resourceComponent->myFileName;
			ourEntityStorage->SaveEntity(anEntity, filePath.GetBuffer());
		}

		ImGui::SameLine();

		if (ImGui::Button("Save As"))
			ImGui::OpenPopup("SaveEntityAs");

		if (ImGui::BeginPopupModal("SaveEntityAs", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetItemDefaultFocus();
			static char filename[64];
			ImGui::InputText("Filename", filename, sizeof(filename));

			if (ImGui::Button("Save", ImVec2(120, 0)))
			{
				if (strlen(filename) > 0)
				{
					FileResoureComponent* resourceComponent = ourEntityStorage->FindComponent<FileResoureComponent>(anEntity);
					FW_ASSERT(resourceComponent != nullptr, "Nullptr ResourceComponent when saving entity, HOW?!");
					resourceComponent->myFileName = filename;
					resourceComponent->myFileName += ".entity";

					FW_String filePath = "Data/Entities/";
					filePath += resourceComponent->myFileName;
					ourEntityStorage->SaveEntity(anEntity, filePath.GetBuffer());

					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}

	void EndEditor()
	{
		if( !ourSelectedEntities.IsEmpty())
		{
			if (FW_Input::WasKeyReleased(FW_Input::KeyCode::DEL))
			{
				for(FW_EntityID entity : ourSelectedEntities)
					ourEntityStorage->QueueEntityRemoval(entity);

				ourSelectedEntities.RemoveAll();
			}
			else
			{
				if(ourSelectedEntities.Count() == 1)
				FW_Editor::EditEntity(ourSelectedEntities[0]);
			}
		}

		if (ImGui::BeginPopup("new_entity_popup"))
		{
			FW_GrowingArray<FW_FileSystem::FileInfo> entityFiles;
			FW_FileSystem::GetAllFilesFromDirectory("Data/Entities", entityFiles);

			ImGui::Text("Entities");
			ImGui::Separator();

			FW_String entity;
			for (const FW_FileSystem::FileInfo& entityFile : entityFiles)
			{
				if (ImGui::Selectable(entityFile.myFileNameNoExtention.GetBuffer()))
				{
					entity = entityFile.myFileNameNoExtention;
					break;
				}
			}

			if (!entity.Empty())
			{
				ourSelectedEntities.RemoveAll();
				for (const Vector2f& position : ourNewEntityPositions)
					ourSelectedEntities.Add(FW_LevelLoader::AddEntity(*ourEntityStorage, position, entity.GetBuffer()));
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		ourEntityStorage = nullptr;
	}

	void SetSelectedEntity(FW_EntityID anEntityID)
	{
		ourSelectedEntities.RemoveAll();

		if(anEntityID != InvalidEntity)
			ourSelectedEntities.Add(anEntityID);
	}

	void SetSelectedEntities(const FW_GrowingArray<FW_EntityID>& someEntities)
	{
		ourSelectedEntities = someEntities;
	}

	void ClearSelectedEntities()
	{
		ourSelectedEntities.RemoveAll();
	}

}