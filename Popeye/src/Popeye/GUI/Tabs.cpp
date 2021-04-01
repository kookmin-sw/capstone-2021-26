#include "Tabs.h"

#include "../GUI/IconsForkAwesome.h"
#include "../GUI/IconsForkAwesomeLargeIcon.h"

#include "../Manager/GUIManager.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../System/FileSystem.h"

#include "../Scene/Scene.h"
#include "../Scene/GameObject.h"

#include "../Component/RenderingComponents.h"
#include "../Resource/Texture.h"
#include "../Resource/Mesh.h"

namespace Popeye{
	extern ImFont	*g_Icon;
	extern FileSystem *g_fileSystem;
	extern ResourceManager *g_ResourceManager;

	extern unsigned int g_SceneView;
	extern unsigned int g_GameView;

	static GameObject *selectedGameObject;
	static Scene *scene;

	//Tab 
	void Tab::SetTab(const char* _name, EventMod _eventmod)
	{
		name = _name;
		eventmod = _eventmod;
	}

	void Tab::CheckHover()
	{
		if (EventManager::GetInstance()->GetState() != eventmod)
		{
			if (ImGui::IsWindowHovered())
			{
				EventManager::GetInstance()->SetState(eventmod);
			}
		}
	}

	void Tab::ShowTab()
	{
		ImGui::Begin(name);

		ShowContents();

		ImGui::End();
	}

	void Tab::ShowContents() { }

	//Tab::sceneView
	void SceneView::ShowContents()
	{
		ImGui::BeginChild("Scene Viewer");

		ImVec2 wsize = ImGui::GetWindowSize();

		ImGui::Image((ImTextureID)g_SceneView, wsize, ImVec2(0, 1), ImVec2(1, 0));

		CheckHover();

		ImGui::EndChild();
	}

	//Tab::GameView
	void GameView::ShowContents()
	{
		ImGui::BeginChild("Game Viewer");

		ImVec2 wsize = ImGui::GetWindowSize();

		ImGui::Image((ImTextureID)g_GameView, wsize, ImVec2(0, 1), ImVec2(1, 0));

		CheckHover();

		ImGui::EndChild();
	}

	//Tab::Hierarchy
	void Hierarchy::ShowContents()
	{
		scene = SceneManager::GetInstance()->currentScene;
		CheckHover();
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		{
			ImGui::OpenPopup("GameObject");
			//popup
		}
		if (ImGui::BeginPopup("GameObject"))
		{
			if (ImGui::Selectable("Create GameObject"))
			{
				scene->CreateGameObject("GameObject");
			}

			ImGui::EndPopup();
		}

		if (ImGui::CollapsingHeader(scene->GetName()))
		{
			for (int i = 0; i < scene->gameObjects.size(); i++)
			{
				if (ImGui::Selectable(scene->gameObjects[i]->GetName().c_str()))
				{
					selectedGameObject = scene->gameObjects[i];
				}
			}
		}
	}

	//Tab::Inspector
	void Inspector::ShowContents()
	{
		static ImGuiTextFilter filter;
		static bool addcomponentCall = false;

		CheckHover();
		if (selectedGameObject) // transform info first
		{
			if (ImGui::CollapsingHeader("Transform"))
			{
				ImGui::DragFloat3("position", (float*)&selectedGameObject->transform.position);
				ImGui::DragFloat3("rotation", (float*)&selectedGameObject->transform.rotation);
				ImGui::DragFloat3("scale", (float*)&selectedGameObject->transform.scale);
			}

			/*Show all Component*/
			int id = selectedGameObject->GetID();
			std::vector<Accessor> accessor = scene->GetAllAddressOfID(id); // TODO::Make copy once.

			for (int i = 0; i < accessor.size(); i++)
			{
				if (accessor[i].componentType != nullptr)
				{
					if (accessor[i].componentType == typeid(Camera).name())
					{
						ShowComponent(selectedGameObject->GetComponent<Camera>());
					}
					if (accessor[i].componentType == typeid(MeshRenderer).name())
					{
						ShowComponent(selectedGameObject->GetComponent<MeshRenderer>());
					}
					if (accessor[i].componentType == typeid(Light).name())
					{
						ShowComponent(selectedGameObject->GetComponent<Light>());
					}
				}
			}

			/*Add Component*/
			if (ImGui::Button("Add Component", ImVec2(ImGui::GetWindowSize().x, 0.0f)))
				addcomponentCall = true;
			
			if (addcomponentCall)
			{
				filter.Draw("search");
				std::vector<const char*> components = {"Camera", "MeshRenderer", "Light"};
				for (int i = 0; i < components.size(); i++)
				{
					if (filter.PassFilter(components[i]))
						if (ImGui::Selectable(components[i]))
						{
							if (i == 0)
								selectedGameObject->AddComponent<Camera>();
							else if (i == 1)
								selectedGameObject->AddComponent<MeshRenderer>();
							else if (i == 2)
								selectedGameObject->AddComponent<Light>();
						}
				}

			}
		}
	}


	void Inspector::ShowComponent(Camera& camera)
	{
		const char* camMod[] = { "Perspective", "Otrhomatric" };

		int cameraMod = -1;
		if (camera.mod == Projection::PERSPECTIVE) { cameraMod = 0; }
		if (camera.mod == Projection::ORTHOGRAPHIC) { cameraMod = 1; }

		if (ImGui::CollapsingHeader("Camera"))
		{
			ImGui::Combo("Projection", &cameraMod, camMod, IM_ARRAYSIZE(camMod), IM_ARRAYSIZE(camMod)); // TODO::Make look pretty.
			if (cameraMod == 0) {
				camera.mod = Projection::PERSPECTIVE;
				ImGui::DragFloat("fov", &camera.fov);
				ImGui::DragFloat("offset x", &camera.offsetX);
				ImGui::DragFloat("offset y", &camera.offsetY);
			}
			if (cameraMod == 1) {
				camera.mod = Projection::ORTHOGRAPHIC;
				ImGui::DragFloat("width", &camera.width);
				ImGui::DragFloat("height", &camera.height);
			}

			ImGui::DragFloat("far view", &camera.farView);
			ImGui::DragFloat("near view", &camera.nearView);
		}
	}

	void Inspector::ShowComponent(MeshRenderer& meshRenderer)
	{
		if (ImGui::CollapsingHeader("MeshRenderer"))
		{
			if (ImGui::TreeNode("Mesh"))
			{
				//Mesh& mesh = MeshRenderer::meshes[meshRenderer.meshIndex];
				ImGui::Text("Mesh");
				ImGui::Selectable("##selectable", false, ImGuiSelectableFlags_SelectOnClick, ImVec2(80.0f, 80.0f));

				//ImGui::Selectable(mesh.id.c_str());
				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsKeyDown(261))
					{
						meshRenderer.meshID = 0;
						meshRenderer.isEmpty = true;
					}
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL"))
					{
						meshRenderer.meshID = *(unsigned int*)payload->Data;
						meshRenderer.isEmpty = false;
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Material"))
			{
				Material& material = MeshRenderer::materials[meshRenderer.materialIndex];

				ImGui::Text("texture");
				ImGui::Selectable("##selectable", false, ImGuiSelectableFlags_SelectOnClick, ImVec2(80.0f, 80.0f));
				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsKeyDown(261))
					{
						material.textureID = -1;
					}
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE"))
					{
						int id = *(const int*)payload->Data;

						//POPEYE_CORE_INFO("received id : {0}", id);

						material.textureID = id;
					}

					ImGui::EndDragDropTarget();
				}
				
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				const ImVec2 p0 = ImGui::GetItemRectMin();
				const ImVec2 p1 = ImGui::GetItemRectMax();

				ImVec2 imgRectMin = ImVec2(p0.x + 2.5f, p0.y + 2.5f);
				ImVec2 imgRectMax = ImVec2(p1.x - 2.5f, p1.y - 2.5f);

				draw_list->AddImage((ImTextureID)material.textureID, imgRectMin, imgRectMax);
				
				ImGui::ColorEdit3("color", (float*)&material.color);
				
				ImGui::DragFloat("ambient", &material.amb_diff_spec[0]);
				ImGui::DragFloat("diffuse", &material.amb_diff_spec[1]);
				ImGui::DragFloat("specular", &material.amb_diff_spec[2]);
				
				ImGui::DragFloat("shininess",	&material.shininess);
				
				ImGui::TreePop();
			}
		}
	}

	void Inspector::ShowComponent(Light& light)
	{
		const char* lighttype[] = { "Point", "Direction", "Spot" };

		int lightMod = -1;
		LightType type = light.ShowLightType();
		
		if (type == LightType::POINT) { lightMod = 0; }
		else if (type == LightType::DIRECTION) { lightMod = 1; }
		else if (type == LightType::SPOT) { lightMod = 2; }

		if (ImGui::CollapsingHeader("Light"))
		{
			ImGui::Combo("Light type", &lightMod, lighttype, IM_ARRAYSIZE(lighttype), IM_ARRAYSIZE(lighttype)); // TODO::Make look pretty.
			if (lightMod == 0)		{ 
				light.ChangeLightType(LightType::POINT);
			}
			else if (lightMod == 1) { 
				light.ChangeLightType(LightType::DIRECTION); 
			}
			else if (lightMod == 2) { 
				light.ChangeLightType(LightType::SPOT);
				
				ImGui::DragFloat("cutOff", &light.cutoff);
				ImGui::DragFloat("outerCutOff", &light.outercutoff);
			}
			
			ImGui::DragFloat("constant", &light.constant);
			ImGui::DragFloat("linear", &light.linear);
			ImGui::DragFloat("quadratic", &light.quadratic);

			ImGui::ColorEdit3("light color", (float*)&light.color);
	
			ImGui::DragFloat("ambient", &light.ambient);
			ImGui::DragFloat("diffuse", &light.diffuse);
			ImGui::DragFloat("specular",&light.specular);
		}
	}

	//Tab::Debug
	void Debug::ShowContents()
	{
		CheckHover();
	}

	//Tab::Project TODO :: Try make code shorter
	void Project::ShowContents()
	{
		static ImVec2 fileSize(70.0f, 80.0f);
		CheckHover();
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiContext& g = *GImGui;
		
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		{
			ImGui::BeginChild("Directories", ImVec2(150, 0), true);
			
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

			if (ImGui::TreeNodeEx((void*)(intptr_t)0, flags, ICON_FK_FOLDER" Root"))
			{
				fs::path currpath = fs::current_path() / "Root";

				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						g_fileSystem->curr_focused_path = fs::current_path() / "Root";
					}
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIR"))
					{
						std::string str(static_cast<char*>(payload->Data), payload->DataSize);

						fs::path path = str;

						if (fs::equivalent(path, g_fileSystem->curr_focused_path))
						{
							g_fileSystem->curr_focused_path = (currpath / path.filename());
						}

						fs::rename(path, (currpath / path.filename()).string());
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
					{
						std::string str(static_cast<char*>(payload->Data), payload->DataSize);

						fs::path path = str;

						fs::rename(path, (currpath / path.filename()).string());
					}

					ImGui::EndDragDropTarget();
				}

				ShowDirectories(0, currpath.string());
				ImGui::TreePop();
			}

			ImGui::EndChild();
		}
		ImGui::SameLine();

		{
			ImGui::BeginChild("Files", ImVec2(0, 0), true);

			std::vector<FileData> dirs, files;
			int totalFileNum = g_fileSystem->ShowFilesAtDir(dirs, files, g_fileSystem->curr_focused_path);

			int i = 0,  fi = 0, di = 0;
			while(i != totalFileNum)
			{
				ImGui::PushID(i);

				ImGui::Selectable("##selectable", false, 0, fileSize);

				FileData filedata;
				if (di < dirs.size())
				{
					filedata = dirs[di];

					if (ImGui::IsItemHovered())
					{
						if (ImGui::IsMouseDoubleClicked(0))
						{
							g_fileSystem->curr_focused_path = filedata.path;
						}
					}
					
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIR"))
						{
							std::string str(static_cast<char*>(payload->Data), payload->DataSize);

							fs::path path = str;

							if (!fs::equivalent(path, filedata.path))
								fs::rename(path, filedata.path / path.filename());
						}
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
						{
							std::string str(static_cast<char*>(payload->Data), payload->DataSize);

							fs::path path = str;

							if (!fs::equivalent(path, filedata.path))
								fs::rename(path, filedata.path / path.filename());
						}

						ImGui::EndDragDropTarget();
					}

					di++;
				}
				else
				{
					filedata = files[fi];
					if (ImGui::IsItemHovered())
					{
						if (ImGui::IsMouseDoubleClicked(0))
						{
							g_fileSystem->ReadFile(filedata);
						}
					}
					fi++;
				}

				std::string fileNamestr = filedata.path.filename().string();
				const char* file_name = fileNamestr.c_str();

				const ImVec2 p0 = ImGui::GetItemRectMin();
				const ImVec2 p1 = ImGui::GetItemRectMax();

				ImDrawList* draw_list = ImGui::GetWindowDrawList();

				float centerPosX = (p0.x + p1.x) * 0.5f;

				ImVec2 imgRectMin = ImVec2(centerPosX - 21.0f, p0.y + 2.0f);
				ImVec2 imgRectMax = ImVec2(centerPosX + 25.0f, imgRectMin.y + 50.0f);

				if (filedata.type == FileType::DIR) { draw_list->AddText(g_Icon, 50.0f, imgRectMin, IM_COL32_WHITE, ICON_FOLDER, 0, fileSize.x); }
				else if (filedata.type == FileType::SOURCE) { draw_list->AddText(g_Icon, 50.0f, imgRectMin, IM_COL32_WHITE, ICON_SOURCE, 0, fileSize.x); }
				else if (filedata.type == FileType::IMAGE) { draw_list->AddText(g_Icon, 50.0f, imgRectMin, IM_COL32_WHITE, ICON_IMAGE, 0, fileSize.x); }
				else if (filedata.type == FileType::MODEL) { draw_list->AddText(g_Icon, 50.0f, imgRectMin, IM_COL32_WHITE, ICON_CUBE, 0, fileSize.x); }
				else  { draw_list->AddText(g_Icon, 50.0f, imgRectMin, IM_COL32_WHITE, ICON_TEXT, 0, fileSize.x); }

				ImVec2 textsize = ImGui::CalcTextSize(file_name);
				float text_x_pos = textsize.x < fileSize.x ? centerPosX - (textsize.x * 0.5f) : centerPosX - (fileSize.x * 0.5f);
				ImVec2 text_pos = ImVec2(text_x_pos, imgRectMax.y);

				draw_list->AddText(g.Font, g.FontSize, text_pos, IM_COL32_WHITE, file_name, 0, fileSize.x);

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					std::string path = filedata.path.string();

					ImGui::SetDragDropPayload("FILE", &path[0], path.size(), ImGuiCond_Once);

					ImGui::Text(file_name);

					ImGui::EndDragDropSource();
				}

				ImGui::PopID();

				float last_button_x2 = p1.x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x; // Expected position if next button was on same line
				if (i + 1 < totalFileNum && next_button_x2 < window_visible_x2)
					ImGui::SameLine();

				i++;
			}
			ImGui::EndChild();
		}
	}

	void Project::ShowDirectories(int id, std::string directory)
	{
		ImGui::PushID(id);

		std::vector<DirectoryData> dirs;
		int dircount = g_fileSystem->ShowDirAtDir(dirs, directory);
		
		ImGui::AlignTextToFramePadding();
		for (int i = 0; i < dircount; i++)
		{
			ImGui::PushID(i);

			fs::path currDir = dirs[i].path;
			std::string str = ICON_FK_FOLDER + currDir.filename().string();
			const char* filename = str.c_str();
			bool is_open = false;

			ImGui::AlignTextToFramePadding();
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (!dirs[i].hasSubDir)
			{
				flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;
				ImGui::TreeNodeEx((void*)(intptr_t)i, flags, filename);
			}
			else
			{
				is_open = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, filename);
			}

			if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseDoubleClicked(0))
				{
					g_fileSystem->curr_focused_path = currDir;
				}
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{

				std::string path = currDir.string();

				ImGui::SetDragDropPayload("DIR", &path[0], path.size(), ImGuiCond_Once);

				ImGui::Text(filename);

				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIR"))
				{
					std::string str(static_cast<char*>(payload->Data), payload->DataSize);

					fs::path path = str;


					if (!fs::equivalent(path, currDir))
					{
						if (fs::equivalent(path, g_fileSystem->curr_focused_path))
						{
							g_fileSystem->curr_focused_path = currDir / path.filename();
						}
						fs::rename(path, currDir / path.filename());
					}
					
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
				{
					std::string str(static_cast<char*>(payload->Data), payload->DataSize);

					fs::path path = str;

					if (!fs::equivalent(path, currDir))
						fs::rename(path, currDir / path.filename());
				}
				ImGui::EndDragDropTarget();
			}

			if (is_open)
			{
				ShowDirectories(i, currDir.string());
				
				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::PopID();
		
	}


	void Resource::ShowContents()
	{
		static bool image_show = false;
		if (ImGui::BeginTabBar("ResourceManager"))
		{
			if (ImGui::BeginTabItem("Texture"))
			{
				if (image_show)
				{
					for (int i = 0; i < g_ResourceManager->textures.size(); i++)
					{
						ImGui::PushID(i);
						ImGui::Selectable("##resource", false, 0, ImVec2(100.0f, 100.0f));
						ImDrawList* draw_list = ImGui::GetWindowDrawList();

						const ImVec2 p0 = ImGui::GetItemRectMin();
						const ImVec2 p1 = ImGui::GetItemRectMax();


						draw_list->AddImage((ImTextureID)g_ResourceManager->textures[i].id, ImVec2(p0.x + 2.0f, p0.y + 2.0f), ImVec2(p1.x - 2.0f, p1.y - 2.0f));


						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							unsigned int id = g_ResourceManager->textures[i].id;

							ImGui::SetDragDropPayload("TEXTURE", &id, sizeof(int), ImGuiCond_Once);


							ImGui::Text("texture : {0}", id);

							ImGui::EndDragDropSource();
						}


						ImGui::PopID();

						ImGui::SameLine();
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Model"))
			{
				if (image_show)
				{
					for (int i = 0; i < g_ResourceManager->meshes.size(); i++)
					{
						ImGui::PushID(i);
						ImGui::Selectable(g_ResourceManager->meshes[i].name.c_str(), false, 0, ImVec2(100.0f, 100.0f));
						ImDrawList* draw_list = ImGui::GetWindowDrawList();

						const ImVec2 p0 = ImGui::GetItemRectMin();
						const ImVec2 p1 = ImGui::GetItemRectMax();


						//draw_list->AddImage((ImTextureID)g_ResourceManager->textures[i].id, ImVec2(p0.x + 2.0f, p0.y + 2.0f), ImVec2(p1.x - 2.0f, p1.y - 2.0f));


						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							ImGui::SetDragDropPayload("MODEL", &i, sizeof(unsigned int), ImGuiCond_Once);

							//ImGui::Text("model : {0}", mesh);

							ImGui::EndDragDropSource();
						}


						ImGui::PopID();

						ImGui::SameLine();
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Material"))
			{

				ImGui::EndTabItem();
			}

			ImGui::InvisibleButton("##empty", ImGui::GetWindowSize());
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
				{
					std::string str(static_cast<char*>(payload->Data), payload->DataSize);

					fs::path path = str;

					POPEYE_CORE_INFO(path.filename().string());

					if (path.extension() == ".jpg" || path.extension() == ".png")
					{
						g_fileSystem->WriteDataToFile("Textures.dat", "Texturestable.dat", path);
					}

					if (path.extension() == ".fbx" || path.extension() == ".obj")
					{
						g_fileSystem->WriteDataToFile("Models.dat", "Modelstable.dat", path);
					}
				}
				ImGui::EndDragDropTarget();
			}


			if (ImGui::Button("Set Resource"))
			{
				g_ResourceManager->SetResources();
				image_show = true;
			}
			ImGui::EndTabBar();
		}

	}

}