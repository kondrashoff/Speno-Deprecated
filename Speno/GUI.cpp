#include "GUI.h"

#include <iostream>
#include <memory>

#include <glad/glad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "SharedEngineData.h"
#include "Timing.h"
#include "Window.h"
#include "ProgramManager.h"
#include "TextureManager.h"
#include "Atmosphere.h"
#include "Camera.h"
#include "Scene.h"
#include "Console.h"
#include "BVH.h"

void GUI::draw() {
	drawMainMenuBar();
	drawInfoWindow();
	drawSettingsWindow();
	drawSceneWindow();
	drawLogWindow();
}

void GUI::drawMainMenuBar() {
	if (BeginMainMenuBar()) {
		if (MenuItem("Exit"))
			Window::Instance.setShouldClose(true);

		if (BeginMenu("Window")) {
			if (MenuItem("Info", "Alt + I"))
				toggleInfoWindow();

			if (MenuItem("Settings", "Alt + S"))
				toggleSettingsWindow();

			if (MenuItem("Scene", "Alt + Shift + S"))
				toggleSceneWindow();

			if (ImGui::MenuItem("Log", "Alt + L"))
				toggleLogWindow();

			EndMenu();
		}

		if (BeginMenu("Display")) {
			if (MenuItem("Full Screen", "F11"))
				Window::Instance.enable(WINDOW_CHANGE_WINDOW_MODE);

			if (MenuItem("Save Screenshot")) {
				int width, height;
				glfwGetFramebufferSize(Window::Instance.getWindow(), &width, &height);

				std::time_t now = std::time(nullptr);
				char timestamp[20];
				std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));

				std::string output_folder = "./Screenshots";
				std::string filename = output_folder + "/screenshot_" + std::string(timestamp) + ".png";

				glPixelStorei(GL_PACK_ALIGNMENT, 1);

				unsigned char* pixels = new unsigned char[width * height * 3];
				glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

				stbi_flip_vertically_on_write(true);

				if (stbi_write_png(filename.c_str(), width, height, 3, pixels, 0)) {
					Console::Instance.push("Successfully saved screenshot to: " + filename);
				}
				else {
					Console::Instance.push("Failed to save screenshot to: " + filename);
				}

				glPixelStorei(GL_PACK_ALIGNMENT, 4);

				delete[] pixels;
			}

			EndMenu();
		}

		EndMainMenuBar();
	}
}

void GUI::drawInfoWindow() {
	if (!m_is_info_window_open) return;

	Begin("Info", &m_is_info_window_open, ImGuiWindowFlags_NoSavedSettings);

	Text("General");

	float timing_avg_ms = Timing::global.average_elapsed;
	float timing_high_ms = Timing::global.highest_elapsed;

	Text("FPS: %.1f", 1000.0f / timing_avg_ms);
	Text("FPS low: %.1f", 1000.0f / timing_high_ms);
	Text("Frame Time: %.1f ms", timing_avg_ms);

	Text("Max Depth: %d", SharedEngineData::Instance.max_depth);

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Maximum number of reflections");
		EndTooltip();
	}

	Spacing();
	Separator();
	Spacing();

	Text("Scene");

	const SceneInfo& scene_info = Scene::Instance.getInfo();
	Text("Meshes: %d", scene_info.number_of_meshes);
	Text("Triangles: %d", scene_info.number_of_triangles);
	Text("Light sources: %d", scene_info.number_of_lights);

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "How many triangles can emit light on the scene");
		EndTooltip();
	}

	Spacing();
	Separator();
	Spacing();

	Text("Camera");

	UniformCamera ucam = Camera::Instance.getUniformCamera();
	Camera& cam = Camera::Instance;

	Text("Position: %.1f %.1f %.1f", ucam.lookfrom.x, ucam.lookfrom.y, ucam.lookfrom.z);
	Text("Rotation: %.1f %.1f %.1f", cam.rotations.x, cam.rotations.y, cam.rotations.z);

	Text("Speed %.1f", cam.speed);

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Units per second");
		EndTooltip();
	}

	Spacing();
	Separator();
	Spacing();

	Text("Atmosphere");

	Text("Rotations: %.0f %.0f %.0f", Atmosphere::Instance.m_pitch, Atmosphere::Instance.m_yaw, Atmosphere::Instance.m_roll);
	Text("Quality %d", Atmosphere::Instance.m_quality_i * Atmosphere::Instance.m_quality_j);

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Number of iterations to approximate the atmosphere");
		EndTooltip();
	}

	End();
}

void GUI::drawSettingsWindow() {
	if (!m_is_settings_window_open) return;
	
	Begin("Settings", &m_is_settings_window_open, ImGuiWindowFlags_NoSavedSettings);

	Text("General");

	SliderInt("Max Depth", &SharedEngineData::Instance.max_depth, 2, 6, "%d");

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Maximum number of reflections");
		EndTooltip();
	}

	Spacing();
	Separator();
	Spacing();

	Text("Camera");

	DragFloat("Speed", &Camera::Instance.speed, 1.0f, 0.004f, 65535.0f, "%.1f", ImGuiSliderFlags_Logarithmic);

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Units per second");
		EndTooltip();
	}

	static bool use_smooth_camera_movement = Camera::Instance.isEnabled(CAMERA_SMOOTH_MOVEMENT);
	if (Checkbox("Smooth Movement", &use_smooth_camera_movement))
		FlagSystem::Instance.camera->set(CAMERA_SMOOTH_MOVEMENT, use_smooth_camera_movement);

	Spacing();
	Separator();
	Spacing();

	Text("Atmosphere");

	SliderFloat("Sun Pitch", &Atmosphere::Instance.m_pitch, 0.0f, 360.0f, "%.0f deg");
	SliderFloat("Sun Yaw", &Atmosphere::Instance.m_yaw, 0.0f, 360.0f, "%.0f deg");
	SliderFloat("Sun Roll", &Atmosphere::Instance.m_roll, 0.0f, 360.0f, "%.0f deg");

	SliderInt("Quality I", &Atmosphere::Instance.m_quality_i, 8, 64, "%d");

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Number of primary iterations to approximate the atmosphere");
		TextColored(TOOLTIP_COLOR_WARNING, "Recommended value is 32");
		EndTooltip();
	}

	SliderInt("Quality J", &Atmosphere::Instance.m_quality_j, 4, 32, "%d");

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Number of secondary iterations to approximate the atmosphere");
		TextColored(TOOLTIP_COLOR_WARNING, "Recommended value is 16");
		EndTooltip();
	}

	Spacing();
	Separator();
	Spacing();

	Text("Temprorary Util");

	static bool use_vsync = false;
	if (Checkbox("Use VSync", &use_vsync)) {
		if (use_vsync) {
			Window::Instance.enable(WINDOW_VERTICAL_SYNC);
		}
		else {
			Window::Instance.disable(WINDOW_VERTICAL_SYNC);
		}
	}

	static bool show_bvh_heatmap = false;
	if (Checkbox("Show BVH heatmap", &show_bvh_heatmap)) {
		if (show_bvh_heatmap) {
			ProgramManager::Instance.disableAll();
			ProgramManager::Instance.get("bvh_heatmap")->enable();
		}
		else {
			ProgramManager::Instance.enableAll();
			ProgramManager::Instance.get("bvh_heatmap")->disable();
		}
	}

	End();
}

void GUI::drawSceneWindow() {
	if (!m_is_scene_window_open) return;

	int width, height;
	glfwGetFramebufferSize(Window::Instance.getWindow(), &width, &height);
	
	float menu_bar_height = GetFontSize() + GetStyle().FramePadding.y * 2.0f;
	float size = (float)width / 4.0f;
	ImVec2 scene_window_pos = ImVec2((float)width - size, menu_bar_height);
	ImVec2 scene_window_size = ImVec2(size, (float)height - menu_bar_height);

	SetNextWindowPos(scene_window_pos);
	SetNextWindowSize(scene_window_size);

	Begin("Scene", &m_is_scene_window_open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	if (Button("Load 3D Model")) {
		Scene::Instance.pushLoadDialog();
	}

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_IMPORTANT, "Only OBJ models (preferably with mtl)");
		EndTooltip();
	}

	SameLine();

	if (Button("Tp to center of scene")) {
		const AABB& root = BVH::Instance.getTLASbounds();
		vec3 center = 0.5f * (root.minimum + root.maximum);

		Camera::Instance.setPosition(center);
	}

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Simply just teleports camera to center of scene");
		EndTooltip();
	}
	
	static bool combine_meshes_on_load = true;
	if (Checkbox("Combine Meshes on load", &combine_meshes_on_load)) {
		Scene::Instance.setCombineMeshes(combine_meshes_on_load);
	}

	if (IsItemHovered()) {
		BeginTooltip();
		TextColored(TOOLTIP_COLOR_BASIC, "Merge all meshes when loaded into 1 mesh");
		TextColored(TOOLTIP_COLOR_DANGER, "Possible very slow performance if meshes are not merged\nalso BVH is not built for unmerged meshes");
		EndTooltip();
	}

	static int selected_material = -1;
	const std::vector<Material>& materials = Scene::Instance.getMaterials();

	if (materials.size() > 0) {

		Spacing();
		Separator();
		Spacing();

		Text("Material Editor");

		if (BeginCombo("Select", selected_material != -1 ? materials[selected_material].name.c_str() : "Select...")) {
			for (int i = 0; i < materials.size(); i++) {
				bool is_selected = selected_material == i;

				if (Selectable(materials[i].name.c_str(), is_selected)) {
					selected_material = i;
				}

				if (is_selected) {
					SetItemDefaultFocus();
				}
			}

			EndCombo();
		}

		if (selected_material != -1) {
			Material* selected = Scene::Instance.getMaterial(selected_material);

			const size_t bufsize = 64;
			char* matname = selected->name.data();
			
			ImGui::InputText("Name", &matname[0], bufsize, ImGuiInputTextFlags_EnterReturnsTrue);

			selected->name = std::string(matname);

			int texdispsize = min(scene_window_size.x, scene_window_size.y);

			if(selected->use_diffuse_tex) {
				int texw, texh;
				selected->diffuse_texture.getSizes(&texw, &texh);

				float aspect = float(texw) / float(texh);
				Image((ImTextureID)selected->diffuse_texture.getID(), ImVec2(texdispsize, texdispsize / aspect));
				
				if (IsItemHovered()) {
					BeginTooltip();
					TextColored(TOOLTIP_COLOR_BASIC, "Color Texture");
					EndTooltip();
				}
			}
			else if (ColorEdit3("Color", &selected->diffuse_color.x)) {
				Scene::Instance.updateMaterial(selected_material);
			}

			if (selected->use_normal_tex) {
				int texw, texh;
				selected->normal_texture.getSizes(&texw, &texh);

				float aspect = float(texw) / float(texh);
				Image((ImTextureID)selected->normal_texture.getID(), ImVec2(texdispsize, texdispsize / aspect));

				if (IsItemHovered()) {
					BeginTooltip();
					TextColored(TOOLTIP_COLOR_BASIC, "Normal Texture");
					EndTooltip();
				}
			}

			if (selected->use_roughness_tex) {
				int texw, texh;
				selected->roughness_texture.getSizes(&texw, &texh);

				float aspect = float(texw) / float(texh);
				Image((ImTextureID)selected->roughness_texture.getID(), ImVec2(texdispsize, texdispsize / aspect));

				if (IsItemHovered()) {
					BeginTooltip();
					TextColored(TOOLTIP_COLOR_BASIC, "Roughness Texture");
					EndTooltip();
				}
			}
			else if (SliderFloat("Roughness", &selected->roughness, 0.0f, 1.0f, "%.2f")) {
				Scene::Instance.updateMaterial(selected_material);
			}

			if (selected->use_emissive_tex) {
				int texw, texh;
				selected->emissive_texture.getSizes(&texw, &texh);

				float aspect = float(texw) / float(texh);
				Image((ImTextureID)selected->emissive_texture.getID(), ImVec2(texdispsize, texdispsize / aspect));

				if (IsItemHovered()) {
					BeginTooltip();
					TextColored(TOOLTIP_COLOR_BASIC, "Emission Texture");
					EndTooltip();
				}
			}
		}
	}

	End();
}

void GUI::drawLogWindow() {
	if (!m_is_log_window_open) return;

	int width, height;
	glfwGetFramebufferSize(Window::Instance.getWindow(), &width, &height);

	float size = (float)height / 4.0f;
	float log_window_width = (float)width;
	if (m_is_scene_window_open) log_window_width -= (float)width / 4.0f;

	ImGui::SetNextWindowPos(ImVec2(0, (float)height - size));
	ImGui::SetNextWindowSize(ImVec2(log_window_width, size));

	ImGui::Begin("Log", &m_is_log_window_open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	Console::Instance.printToGUI();
	ImGui::End();
}

void GUI::initialize() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	CreateContext();

	ImGuiIO& io = GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(Window::Instance.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460 core");

	StyleColorsDark();
	ImGuiStyle& style = GetStyle();

	// Base interface colors
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.14f, 0.12f, 1.00f);

	// Main accents
	style.Colors[ImGuiCol_Button] = ImVec4(0.86f, 0.44f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.59f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.94f, 0.48f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.96f, 0.55f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.59f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.63f, 0.33f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.46f, 0.02f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.96f, 0.55f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.96f, 0.55f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.55f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.59f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.75f, 0.37f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.22f, 0.11f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.14f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.86f, 0.44f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.98f, 0.59f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.94f, 0.48f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.14f, 0.12f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.86f, 0.44f, 0.00f, 0.50f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.46f, 0.43f, 0.50f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.36f, 0.10f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.40f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.59f, 0.26f, 0.20f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.57f, 0.26f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.52f, 0.26f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.38f, 0.18f, 0.86f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.50f, 0.26f, 0.80f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.34f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.11f, 0.07f, 0.97f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.24f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.19f, 0.19f, 1.00f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.32f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.24f, 0.23f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.47f, 0.26f, 0.35f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.52f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.69f, 0.59f, 0.47f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.64f, 0.51f, 0.41f, 0.69f);

	// Sizes and roundings
	style.WindowPadding = ImVec2(8, 8);
	style.WindowRounding = 7.0f;
	style.FramePadding = ImVec2(4, 3);
	style.FrameRounding = 5.0f;
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.IndentSpacing = 25.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 3.0f;

	// Other settings
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.TabBorderSize = 1.0f;

	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
}

void GUI::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();
	draw();
	Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
}

void GUI::toggleInfoWindow() {
	m_is_info_window_open = !m_is_info_window_open;
}

void GUI::toggleSettingsWindow() {
	m_is_settings_window_open = !m_is_settings_window_open;
}

void GUI::toggleSceneWindow() {
	m_is_scene_window_open = !m_is_scene_window_open;
}

void GUI::toggleLogWindow() {
	m_is_log_window_open = !m_is_log_window_open;
}

GUI::~GUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	DestroyContext();
}