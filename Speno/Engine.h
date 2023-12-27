#pragma once

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <Windows.h>
#include <commdlg.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "PerformanceMonitor.h"
#include "EngineData.h"
#include "Mesh.h"
#include "Camera.h"
#include "Quad.h"
#include "Shader.h"

class Engine {
public:

	Engine() = default;

	void init(bool fullscreen = false) {
		initGLFW();
		
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		initWindow(fullscreen);
		initOpenGL();
		initImGui();

		stbi_set_flip_vertically_on_load(1);
		stbi_flip_vertically_on_write(1);

		screen_quad.init();
		main_program.init("main.glsl");
		denoising_program.init("denoiser.glsl");
		postprocess_program.init("postprocess.glsl");
		output_program.init("output.glsl");

		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedEngineData), &shared_engine_data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		GLuint uboIndex = glGetUniformBlockIndex(main_program.id, "SharedEngineData");
		glUniformBlockBinding(main_program.id, uboIndex, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

		main_program.createGBuffers(window_width, window_height, GL_RGBA32F, GL_LINEAR, GL_CLAMP_TO_EDGE);
		denoising_program.createGBuffers(window_width, window_height, GL_RGBA32F, GL_LINEAR, GL_CLAMP_TO_EDGE);
		postprocess_program.createFBO(window_width, window_height, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);

		denoising_program.bindTexture(0, main_program.fbo_texture, "gbufferDiffuse");
		denoising_program.bindTexture(1, main_program.gbuffer_position_texture, "gbufferPosition");
		denoising_program.bindTexture(2, main_program.gbuffer_normal_texture, "gbufferNormal");
		main_program.bindTexture(3, denoising_program.fbo_texture, "gbufferPreviousDiffuse");
		main_program.bindTexture(4, denoising_program.gbuffer_position_texture, "gbufferPreviousPosition");
		postprocess_program.bindTexture(5, denoising_program.gbuffer_albedo_texture, "denoisedImage");
		postprocess_program.bindTexture(6, main_program.gbuffer_albedo_texture, "gbufferAlbedo");
		output_program.bindTexture(7, postprocess_program.fbo_texture, "postprocessedFrame");

		setupSTBN();
		camera.init(glm::vec3(0.0f), 60.0f, 10.0f, 3);
	}

	void run() {
		processEvents();
		while (!glfwWindowShouldClose(window)) {
			newFrame();

			main_program.use();
			screen_quad.draw();
			main_program.unuse();

			denoising_program.use();
			screen_quad.draw();
			denoising_program.unuse();

			postprocess_program.use();
			screen_quad.draw();
			postprocess_program.unuse();

			output_program.use();
			screen_quad.draw();
			output_program.unuse();

			renderGUI();

			glfwSwapBuffers(window);
			processEvents();
		}
	};

	~Engine() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
	};

private:

	void renderGUI() {
		ImGuiWindowFlags window_flags_info = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
		ImGuiWindowFlags window_flags_log = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
		ImGuiWindowFlags window_flags_scene = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::MenuItem("Exit")) {
				glfwSetWindowShouldClose(window, true);
			}

			if (ImGui::BeginMenu("Window")) {
				if (ImGui::MenuItem("Info")) {
					is_info_window_open = true;
				}

				if (ImGui::MenuItem("Settings")) {
					is_settings_window_open = true;
				}

				if (ImGui::MenuItem("Scene")) {
					is_scene_window_open = true;
				}

				if (ImGui::MenuItem("Log")) {
					is_log_window_open = true;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Display")) {
				if (ImGui::MenuItem("Save Screenshot", "Print Screen")) {
					saveScreenshot();
				}

				if (ImGui::MenuItem("Full Screen", "F11")) {
					toggleFullscreen();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (is_info_window_open) {
			glm::vec3 lookfrom = camera.getCameraPosition();

			ImGui::Begin("Info", &is_info_window_open, window_flags_info);

			ImGui::Text("FPS: %.1f", 1.0f / shared_engine_data.delta_time);
			ImGui::Text("Camera position: %.1f, %.1f, %.1f", lookfrom.x, lookfrom.y, lookfrom.z);
			ImGui::Text("Camera pitch: %.1f", camera.pitch);
			ImGui::Text("Camera yaw: %.1f", camera.yaw);
			ImGui::Text("Camera speed: %.1f", camera.speed);

			ImGui::End();
		}

		if (is_settings_window_open) {
			ImGui::Begin("Settings", &is_settings_window_open, window_flags_info);
			
			ImGui::Separator(); ImGui::Spacing();

			ImGui::Text("General:");
			ImGui::Checkbox("Vertical Sync", &is_vertical_sync_enabled);
			ImGui::Checkbox("Two Sided Geometry", &shared_engine_data.use_two_sided_geometry);
			if (!shared_engine_data.use_pathtracing) {
				ImGui::Checkbox("Shadows", &shared_engine_data.use_shadows);
				ImGui::Checkbox("WIP PTAO", &shared_engine_data.use_pathtracing_ambient_occlusion);
			}
			ImGui::Checkbox("Path Tracing", &shared_engine_data.use_pathtracing);
			if (shared_engine_data.use_pathtracing || shared_engine_data.use_pathtracing_ambient_occlusion) {
				ImGui::Checkbox("WIP Temporal Reprojection", &shared_engine_data.use_temporal_reprojection);
				
				if (shared_engine_data.use_temporal_reprojection) {
					ImGui::Checkbox("Do not reproject if the camera is moving.", &shared_engine_data.do_not_reproject_movement);
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.3f), 
							"Because temporal reprojection is still in development,"
							"I have not yet managed to somehow fix the fact that the picture gets worse when the camera moves.");
						ImGui::EndTooltip();
					}
				}
			}

			ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

			ImGui::Text("Camera:");
			ImGui::SliderFloat("Speed", &camera.speed, 1.0f, 100.0f, "%.1f", 0);
			ImGui::SliderFloat("Fov", &camera.fov, 10.0f, 140.0f, "%.f", 0);
			if (shared_engine_data.use_pathtracing) ImGui::SliderInt("Max Depth", &camera.max_depth, 3, 16, "%d", 0);

			ImGui::Spacing(); ImGui::Separator();

			ImGui::End();
		}

		if (is_scene_window_open) {
			float menu_bar_height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;

			float size = (float)window_width / 4.0f;
			ImVec2 window_pos = ImVec2((float)window_width - size, menu_bar_height);
			ImVec2 window_size = ImVec2(size, (float)window_height - menu_bar_height);

			ImGui::SetNextWindowPos(window_pos);
			ImGui::SetNextWindowSize(window_size);

			ImGui::Begin("Scene", &is_scene_window_open, window_flags_scene);

			if (ImGui::Button("Load HDRI")) {
				wchar_t filePath[MAX_PATH] = { 0 };
				HWND hwnd = glfwGetWin32Window(window);
				if (OpenHDRiFileDialog(hwnd, filePath, MAX_PATH)) {
					char utf8FilePath[MAX_PATH];
					WideCharToMultiByte(CP_UTF8, 0, filePath, -1, utf8FilePath, MAX_PATH, NULL, NULL);
					std::string filePathStr(utf8FilePath);
					main_program.hdriLoadFromFile(18, filePathStr, "hdri_texture", 19, "hdri_data_texture");
					shared_engine_data.use_hdri = true;
					glfwRestoreWindow(window);
				}
				glfwRestoreWindow(window);
			}

			ImGui::SameLine();

			if (ImGui::Button("Load Mesh")) {
				wchar_t filePath[MAX_PATH] = { 0 };
				HWND hwnd = glfwGetWin32Window(window);
				if (OpenMeshFileDialog(hwnd, filePath, MAX_PATH)) {
					char utf8FilePath[MAX_PATH];
					WideCharToMultiByte(CP_UTF8, 0, filePath, -1, utf8FilePath, MAX_PATH, NULL, NULL);
					std::string filePathStr(utf8FilePath);
					should_update_mesh = true;
					glfwRestoreWindow(window);
					mesh_path = filePathStr;
				}
				glfwRestoreWindow(window);
			}

			ImGui::SameLine();

			if (ImGui::Button("Load Mesh Texture")) {
				wchar_t filePath[MAX_PATH] = { 0 };
				HWND hwnd = glfwGetWin32Window(window);
				if (OpenImageFileDialog(hwnd, filePath, MAX_PATH)) {
					char utf8FilePath[MAX_PATH];
					WideCharToMultiByte(CP_UTF8, 0, filePath, -1, utf8FilePath, MAX_PATH, NULL, NULL);
					std::string filePathStr(utf8FilePath);
					main_program.textureLoadFromFile(17, filePathStr, "mesh_texture", GL_LINEAR, GL_REPEAT);
					shared_engine_data.mesh_color_method = 0;
					glfwRestoreWindow(window);
				}
				glfwRestoreWindow(window);
			}

			static const char* mesh_color_methods[] = { "Texture", "Single color", "Automatic landscape color", "Atrribute color" };
			ImGui::Combo("Color Method", &shared_engine_data.mesh_color_method, mesh_color_methods, IM_ARRAYSIZE(mesh_color_methods));
			if (shared_engine_data.mesh_color_method == 0 && !is_mesh_texture_loaded) {
				shared_engine_data.mesh_color_method = 1;
				is_log_window_open = true;
				console_buffer += "Failed to load texture. Please load the mesh texture first.\n";
			}
			else if (shared_engine_data.mesh_color_method == 1) {
				ImGui::ColorEdit3("Color", &shared_engine_data.mesh_color.x);
			}

			ImGui::End();
		}

		if (is_log_window_open) {
			float size = (float)window_height / 4.0f;
			float log_window_width = (float)window_width;
			if(is_scene_window_open) log_window_width -= (float)window_width / 4.0f;

			ImGui::SetNextWindowPos(ImVec2(0, (float)window_height - size));
			ImGui::SetNextWindowSize(ImVec2(log_window_width, size));

			ImGui::Begin("Log", &is_log_window_open, window_flags_log);
			ImGui::Text(console_buffer.c_str());
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	inline void setupSTBN() {
		stbn_width = 128;
		stbn_height = 128;
		stbn_depth = 128;

		main_program.setUniform3i("stbn_resolution", glm::ivec3(stbn_width, stbn_height, stbn_depth));

		loadSTBN(8, "stbn_scalar", "STBN/scalar/stbn_scalar_2Dx1Dx1D_128x128x128x1", stbn_scalar_texture_array);
        loadSTBN(9, "stbn_vec1", "STBN/vec1/stbn_vec1_2Dx1D_128x128x128", stbn_vec1_texture_array);
		loadSTBN(10, "stbn_unitvec1", "STBN/vec1/unit/stbn_unitvec1_2Dx1D_128x128x128", stbn_unitvec1_texture_array);
		loadSTBN(11, "stbn_vec2", "STBN/vec2/stbn_vec2_2Dx1D_128x128x128", stbn_vec2_texture_array);
		loadSTBN(12, "stbn_unitvec2", "STBN/vec2/unit/stbn_unitvec2_2Dx1D_128x128x128", stbn_unitvec2_texture_array);
		loadSTBN(13, "stbn_vec3", "STBN/vec3/stbn_vec3_2Dx1D_128x128x128", stbn_vec3_texture_array);
		loadSTBN(14, "stbn_unitvec3", "STBN/vec3/unit/stbn_unitvec3_2Dx1D_128x128x128", stbn_unitvec3_texture_array);
		loadSTBN(15, "stbn_unitvec3_cosine", "STBN/vec3/unit/cosine/stbn_unitvec3_cosine_2Dx1D_128x128x128", stbn_unitvec3_cosine_texture_array);
		loadSTBN(16, "stbn_unitvec3_hdri", "STBN/vec3/unit/hdri/starmap/stbn_unitvec3_starmap_2Dx1D_128x128x128", stbn_unitvec3_hdri_texture_array);
	}

	void loadSTBN(GLuint texture_unit, std::string uniform_name, std::string filepath, GLuint &stbn_texture_array) {
		glGenTextures(1, &stbn_texture_array);
		glBindTexture(GL_TEXTURE_2D_ARRAY, stbn_texture_array);

		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16F, stbn_width, stbn_height, stbn_depth);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

		for (int i = 0; i < stbn_depth; i++) {
			std::string stbn_filename = filepath + "_" + std::to_string(i) + ".png";
			int stbn_width, stbn_height, stbn_num_channels;
			stbi_us* stbn_texture = stbi_load_16(stbn_filename.c_str(), &stbn_width, &stbn_height, &stbn_num_channels, 0);

			if (!stbn_texture) {
				printf("Failed to load stbn texture: %s\n", stbn_filename.c_str());
				exit(EXIT_FAILURE);
			}

			if (stbn_num_channels == 1) {
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, stbn_width, stbn_height, 1, GL_RED, GL_UNSIGNED_SHORT, stbn_texture);
			}
			else if (stbn_num_channels == 4) {
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, stbn_width, stbn_height, 1, GL_RGBA, GL_UNSIGNED_SHORT, stbn_texture);
			}
			else {
				printf("Unexpected number of channels in stbn texture: %s\n", stbn_filename.c_str());
				exit(EXIT_FAILURE);
			}

			stbi_image_free(stbn_texture);
		}

		main_program.bindTexture(texture_unit, stbn_texture_array, uniform_name, GL_TEXTURE_2D_ARRAY);
	}

	void processEvents() {
		if (should_update_mesh) {
			scene.loadFromFile(mesh_path);
			should_update_mesh = false;
		}

		float new_time = (float)glfwGetTime();
		shared_engine_data.delta_time = new_time - shared_engine_data.time;
		shared_engine_data.time = new_time;
		shared_engine_data.frame++;

		glfwPollEvents();
		cameraEvent();

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SharedEngineData), &shared_engine_data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		main_program.setUniform2f("resolution", glm::vec2(window_width, window_height));
		denoising_program.setUniform1b("use_pathtracing_ambient_occlusion", shared_engine_data.use_pathtracing_ambient_occlusion);
		denoising_program.setUniform1b("use_pathtracing", shared_engine_data.use_pathtracing);

		glfwSwapInterval((int)is_vertical_sync_enabled);
	}

	void cameraEvent() {
		camera.beforeMovement();

		float speed = shared_engine_data.delta_time;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= 3.146f;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed *= 0.318f;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera.stepForward(speed);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera.stepLeft(speed);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera.stepRight(speed);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera.stepBack(speed);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			camera.stepDown(speed);
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			camera.stepUp(speed);
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
			if (is_camera_move_enabled) {
				double mouse_xpos, mouse_ypos;
				glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos);

				int centerX = window_width / 2;
				int centerY = window_height / 2;

				float x_velocity = (centerX - (float)mouse_xpos) / window_width * 90.0f;
				float y_velocity = (centerY - (float)mouse_ypos) / window_height * 90.0f;

				camera.pitch += y_velocity;
				camera.pitch = std::clamp(camera.pitch, -90.0f, 90.0f);
				camera.yaw += x_velocity;

				glfwSetCursorPos(window, centerX, centerY);
			}
			else {
				int centerX = window_width / 2;
				int centerY = window_height / 2;

				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPos(window, centerX, centerY);
				is_camera_move_enabled = true;
			}
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			is_camera_move_enabled = false;
		}

		camera.update();
	}

	inline void initWindow(bool fullscreen) {
		std::string title = "Speno " + std::to_string(SPENO_VERSION_MAJOR) + "." + std::to_string(SPENO_VERSION_MINOR) + "." + std::to_string(SPENO_VERSION_PATCH) + "." + std::to_string(SPENO_VERSION_TWEAK);
		
		if (fullscreen) {
			GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);

			window_width = video_mode->width;
			window_height = video_mode->height;

			last_window_width = window_width;
			last_window_height = window_height;

			window = glfwCreateWindow(window_width, window_height, title.c_str(), primary_monitor, NULL);
		}
		else {
			window_width = 640;
			window_height = 480;

			last_window_width = window_width;
			last_window_height = window_height;

			window = glfwCreateWindow(window_width, window_height, title.c_str(), nullptr, NULL);
		}

		if (!window) {
			glfwTerminate();

			std::cout << "Failed to create window or OpenGL context.";
			exit(EXIT_FAILURE);
		}

		int width, height, channels;
		unsigned char *data = stbi_load("Texture/Icon/icon_48x48.png", &width, &height, &channels, 0);
		if (!data) {
			std::cout << "Failed to load icon.";
			exit(EXIT_FAILURE);
		}

		GLFWimage icon;
		icon.width = width;
		icon.height = height;
		icon.pixels = data;

		glfwSetWindowIcon(window, 1, &icon);
		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, this);
		glfwSwapInterval((int)is_vertical_sync_enabled);
		glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
		glfwSetKeyCallback(window, keyCallback);
	}

	inline void initImGui() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460 core");
	}

	inline void initOpenGL() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD.";
			exit(EXIT_FAILURE);
		}
	}

	inline void initGLFW() {
		if (!glfwInit()) {
			std::cout << "Failed to initialize GLFW.";
			exit(EXIT_FAILURE);
		}

		glfwSetErrorCallback(errorCallback);
	}

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
		if (engine != nullptr) {
			if (action == GLFW_PRESS) {
				if (key == GLFW_KEY_ESCAPE) {
					glfwSetWindowShouldClose(window, true);
				}

				else if (key == GLFW_KEY_F11) {
					engine->toggleFullscreen();
				}

				if (key == GLFW_KEY_PRINT_SCREEN) {
					engine->saveScreenshot();
				}
			}
		}
	}

	bool OpenHDRiFileDialog(HWND hwndOwner, wchar_t* buffer, DWORD bufferSize) {
		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndOwner;
		ofn.lpstrFile = buffer;
		ofn.lpstrFile[0] = L'\0';
		ofn.nMaxFile = bufferSize;
		ofn.lpstrFilter = L"HDR Files\0*.hdr\0All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = L"hdr";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.lpstrInitialDir = L"./Texture/Hdri";

		if (!GetOpenFileNameW(&ofn)) {
			std::string err = std::to_string(CommDlgExtendedError());
			if(err != "0") console_buffer += "Failed to open file dialog. " + err + "\n";
			return false;
		}

		return true;
	}

	bool OpenMeshFileDialog(HWND hwndOwner, wchar_t* buffer, DWORD bufferSize) {
		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndOwner;
		ofn.lpstrFile = buffer;
		ofn.lpstrFile[0] = L'\0';
		ofn.nMaxFile = bufferSize;
		ofn.lpstrFilter = L"OBJ Files\0*.obj\0All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = L"obj";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.lpstrInitialDir = L"./Meshes";

		if (!GetOpenFileNameW(&ofn)) {
			std::string err = std::to_string(CommDlgExtendedError());
			if (err != "0") console_buffer += "Failed to open file dialog. " + err + "\n";
			return false;
		}

		return true;
	}

	bool OpenImageFileDialog(HWND hwndOwner, wchar_t* buffer, DWORD bufferSize) {
		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndOwner;
		ofn.lpstrFile = buffer;
		ofn.lpstrFile[0] = L'\0';
		ofn.nMaxFile = bufferSize;
		ofn.lpstrFilter = L"PNG Files\0*.png\0All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = L"png";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.lpstrInitialDir = L"./Texture";

		if (!GetOpenFileNameW(&ofn)) {
			std::string err = std::to_string(CommDlgExtendedError());
			if (err != "0") console_buffer += "Failed to open file dialog. " + err + "\n";
			return false;
		}

		return true;
	}

	void saveScreenshot() {
		std::string output_folder = "Screenshots";

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		unsigned char* pixels = new unsigned char[width * height * 3];

		postprocess_program.use();
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		postprocess_program.unuse();

		std::time_t now = std::time(nullptr);
		char timestamp[20];
		std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));

		std::string filename = output_folder + "/screenshot_" + std::string(timestamp) + ".png";

		if (stbi_write_png(filename.c_str(), width, height, 3, pixels, 0)) {
			console_buffer += "Successfully saved screenshot to: " + filename + "\n";
		}
		else {
			console_buffer += "Failed to save screenshot to: " + filename + "\n";
		}

		delete[] pixels;
	}

	void toggleFullscreen() {
		GLFWmonitor* monitor = glfwGetWindowMonitor(window);
		if (monitor == nullptr) {
			// Save the current window position and size to restore later
			glfwGetWindowPos(window, &last_window_x, &last_window_y);
			glfwGetWindowSize(window, &last_window_width, &last_window_height);

			// Get the primary monitor and its video mode
			monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);

			// Switch to fullscreen
			glfwSetWindowMonitor(window, monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
		}
		else {
			// Restore to windowed mode using the last saved position and size
			glfwSetWindowMonitor(window, nullptr, last_window_x, last_window_y, last_window_width, last_window_height, 0);
		}
	}

	static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
		if (width == 0 || height == 0) {
			return;
		}

		Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
		if (engine != nullptr) {
			glViewport(0, 0, width, height);
			engine->main_program.resize(width, height);
			engine->denoising_program.resize(width, height);
			engine->postprocess_program.resize(width, height);
			engine->main_program.setUniform2f("resolution", glm::vec2(width, height));
			engine->window_width = width;
			engine->window_height = height;
			engine->denoising_program.bindTexture(0, engine->main_program.fbo_texture, "gbufferDiffuse");
		}
	}

	static void errorCallback(int error, const char* description) {
		fprintf(stderr, "GLFW Error: %s\n", description);
		exit(EXIT_FAILURE);
	}

	inline void newFrame() {
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClear(GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

private:

	int window_width, window_height, window_x, window_y;
	int last_window_width, last_window_height, last_window_x = 0, last_window_y = 0;

	int stbn_width, stbn_height, stbn_depth;
	
	bool should_update_mesh = false;
	std::string mesh_path;

	bool is_info_window_open = false;
	bool is_settings_window_open = false;
	bool is_scene_window_open = false;
	bool is_log_window_open = false;
	bool is_camera_move_enabled = false;
	bool is_vertical_sync_enabled = false;
	bool is_mesh_texture_loaded = false;
	
	GLuint ubo;
	GLuint stbn_scalar_texture_array;
	GLuint stbn_vec1_texture_array;
	GLuint stbn_vec2_texture_array;
	GLuint stbn_vec3_texture_array;
	GLuint stbn_unitvec1_texture_array;
	GLuint stbn_unitvec2_texture_array;
	GLuint stbn_unitvec3_texture_array;
	GLuint stbn_unitvec3_cosine_texture_array;
	GLuint stbn_unitvec3_hdri_texture_array;

	GLFWwindow* window;
	Mesh scene;
	Camera camera;
	Quad screen_quad;
	ShaderProgram main_program;
	ShaderProgram denoising_program;
	ShaderProgram postprocess_program;
	ShaderProgram output_program;
};