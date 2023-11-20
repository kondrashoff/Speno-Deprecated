#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Denoiser.h"
#include "Mesh.h"
#include "Camera.h"
#include "Sky.h"
#include "BlockWorld.h"

class Engine {
public:

	Engine() {
		if (!glfwInit()) {
			std::cerr << "Failed to initialize GLFW.";
			exit(EXIT_FAILURE);
		}

		glfwSetErrorCallback(errorCallback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		primary_monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);

		window = glfwCreateWindow(video_mode->width, video_mode->height, "Engine", primary_monitor, NULL);
		if (!window) {
			std::cerr << "Failed to create window or OpenGL context.";
			exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		glfwSetWindowUserPointer(window, this);
		glfwSetWindowFocusCallback(window, windowFocusCallbackStatic);
		glfwSetKeyCallback(window, keyCallbackStatic);
		glfwSetFramebufferSizeCallback(window, framebufferSizeCallbackStatic);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

		GLenum err = glewInit();
		if (err != GLEW_OK) {
			std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err);
			exit(EXIT_FAILURE);
		}

		std::string vertex_shader_source = readShaderFile("vertex.glsl");
		std::string main_shader_source = readShaderFile("main.glsl");
		std::string postprocess_shader_source = readShaderFile("postprocess.glsl");

		std::regex include_pattern("#include \"([^\"]+)\"");
		std::smatch include_matches;

		while (std::regex_search(main_shader_source, include_matches, include_pattern)) {
			std::string include_directive = include_matches[0];
			std::string include_file_name = include_matches[1];

			std::string include_source = readShaderFile(include_file_name);

			main_shader_source.replace(include_matches.position(0), include_directive.length(), include_source + "\n");
		}

#ifdef DEBUG
		std::istringstream f(main_shader_source);
		std::string line;
		int line_num = 0;
		while (std::getline(f, line)) {
			std::cout << ++line_num << ". " << line << std::endl;
		}
#endif

		main_program = createShaderProgram(vertex_shader_source, main_shader_source);
		postprocess_program = createShaderProgram(vertex_shader_source, postprocess_shader_source);

		glUseProgram(main_program);

		glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
		glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		pathtracing_denoiser = Denoiser(width, height);

		setupCameraSSBO();
		setupOldCameraSSBO();
		setupSkySSBO();

		glGenTextures(1, &map_texture);
		glBindTexture(GL_TEXTURE_2D, map_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		int texture_width, texture_height, num_channels;
		unsigned char* data = stbi_load("Textures/heightmaps/heightmap Mountain 3.png", &texture_width, &texture_height, &num_channels, STBI_rgb_alpha);
		if (!data) {
			std::cerr << "Failed to load texture: " << "map" << std::endl;
			exit(EXIT_FAILURE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &stone_texture);
		glBindTexture(GL_TEXTURE_2D, stone_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		data = stbi_load("Textures/hardened_clay.png", &texture_width, &texture_height, &num_channels, STBI_rgb);
		if (!data) {
			std::cerr << "Failed to load texture: " << "stone.png" << std::endl;
			exit(EXIT_FAILURE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &grass_top_texture);
		glBindTexture(GL_TEXTURE_2D, grass_top_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		data = stbi_load("Textures/grass_top.png", &texture_width, &texture_height, &num_channels, STBI_rgb);
		if (!data) {
			std::cerr << "Failed to load texture: " << "grass_top.png" << std::endl;
			exit(EXIT_FAILURE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glGenTextures(1, &grass_side_texture);
		glBindTexture(GL_TEXTURE_2D, grass_side_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		data = stbi_load("Textures/grass_side.png", &texture_width, &texture_height, &num_channels, STBI_rgb);
		if (!data) {
			std::cerr << "Failed to load texture: " << "grass_side.png" << std::endl;
			exit(EXIT_FAILURE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &dirt_texture);
		glBindTexture(GL_TEXTURE_2D, dirt_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		data = stbi_load("Textures/dirt.png", &texture_width, &texture_height, &num_channels, STBI_rgb);
		if (!data) {
			std::cerr << "Failed to load texture: " << "dirt.png" << std::endl;
			exit(EXIT_FAILURE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		resolution_location = glGetUniformLocation(main_program, "u_resolution");
		frame_location      = glGetUniformLocation(main_program, "u_frame");
		frame_seed_location = glGetUniformLocation(main_program, "u_frame_seed");
		time_location       = glGetUniformLocation(main_program, "u_time");
		delta_time_location = glGetUniformLocation(main_program, "u_delta_time");

		glUniform2f(resolution_location, static_cast<float>(width), static_cast<float>(height));

		glGenTextures(1, &diffuse_texture);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &albedo_texture);
		glBindTexture(GL_TEXTURE_2D, albedo_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &normal_texture);
		glBindTexture(GL_TEXTURE_2D, normal_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &position_texture);
		glBindTexture(GL_TEXTURE_2D, position_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture,  0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, albedo_texture,   0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normal_texture,   0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, position_texture, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Failed to create framebuffer: " << status;
			exit(EXIT_FAILURE);
		}

		const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, draw_buffers);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(0);
	}

	void setScene(Mesh &other_scene) {
		scene = other_scene;
		glUseProgram(main_program);
		setupTrianglesSSBO();
		setupBVHSSBO();
	}

	void generateChunks(unsigned int seed = 0) {
		Perfomance chunks_perf_monitor;

		chunks_perf_monitor.start();
		for (int x = 0; x < world.generation_distance; x++) {
			for (int z = 0; z < world.generation_distance; z++) {
				Perfomance chunk_perf_monitor;
				chunk_perf_monitor.start();

				Chunk generated_chunk;
				generated_chunk.generate(seed, x * 16, z * 16);
				world.chunks.push_back(generated_chunk);

				chunk_perf_monitor.stop();
				std::cout << "Chunk[" << x << "][" << z << "] generated. " << chunk_perf_monitor << std::endl;
			}
		}
		chunks_perf_monitor.stop();

		std::cout << "Chunks generated. " << chunks_perf_monitor << std::endl;

		setupChunksSSBO();
	}

	void run() {
		while (!glfwWindowShouldClose(window)) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(main_program);

			time = (float)glfwGetTime();
			camera.buildFromRotations();

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_camera);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Camera), &camera);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_old_camera);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Camera), &old_camera);

			glUniform1ui(frame_location, frame);
			glUniform1ui(frame_seed_location, rand());
			glUniform1f(time_location, time);
			glUniform1f(delta_time_location, delta_time);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuse_texture);
			glUniform1i(glGetUniformLocation(main_program, "previous_diffuse_texture"), 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, albedo_texture);
			glUniform1i(glGetUniformLocation(main_program, "previous_albedo_texture"), 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, normal_texture);
			glUniform1i(glGetUniformLocation(main_program, "previous_normal_texture"), 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, position_texture);
			glUniform1i(glGetUniformLocation(main_program, "previous_position_texture"), 3);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, map_texture);
			glUniform1i(glGetUniformLocation(main_program, "map_texture"), 4);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, stone_texture);
			glUniform1i(glGetUniformLocation(main_program, "stone_texture"), 5);

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, grass_top_texture);
			glUniform1i(glGetUniformLocation(main_program, "grass_top_texture"), 6);

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, grass_side_texture);
			glUniform1i(glGetUniformLocation(main_program, "grass_side_texture"), 7);

			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, dirt_texture);
			glUniform1i(glGetUniformLocation(main_program, "dirt_texture"), 8);

			glEnable(GL_DEPTH_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

			glBegin(GL_TRIANGLES);
			glVertex3f(-1.0f, -1.0f, 0.0f);
			glVertex3f(-1.0f, 1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glVertex3f(-1.0f, 1.0f, 0.0f);
			glVertex3f(1.0f, 1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glEnd();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);

			glUseProgram(postprocess_program);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuse_texture);
			glUniform1i(glGetUniformLocation(postprocess_program, "diffuse_texture"), 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, albedo_texture);
			glUniform1i(glGetUniformLocation(postprocess_program, "albdeo_texture"), 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, normal_texture);
			glUniform1i(glGetUniformLocation(postprocess_program, "normal_texture"), 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, position_texture);
			glUniform1i(glGetUniformLocation(postprocess_program, "position_texture"), 3);

			glBegin(GL_TRIANGLES);
			glVertex3f(-1.0f, -1.0f, 0.0f);
			glVertex3f(-1.0f, 1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glVertex3f(-1.0f, 1.0f, 0.0f);
			glVertex3f(1.0f, 1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glEnd();

			glUseProgram(0);

			old_camera = camera;

			processCameraMovement();

			if (is_rendering) frame++;

			glfwSwapBuffers(window);
			glfwPollEvents();

			delta_time = (float)glfwGetTime() - time;
		}
	}

	~Engine() {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		glfwDestroyWindow(window);
		glfwTerminate();

		std::cerr << "position: " << camera.lookfrom << " pitch: " << camera.pitch << " yaw: " << camera.yaw << std::endl;

		std::cerr << "Engine process ended successfully!";
	}

private:

	void setupCameraSSBO() {
		glGenBuffers(1, &ssbo_camera);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_camera);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Camera), &camera, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_camera);
	}
	
	void setupOldCameraSSBO() {
		glGenBuffers(1, &ssbo_old_camera);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_old_camera);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Camera), &old_camera, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_old_camera);
	}
	
	void setupSkySSBO() {
		glGenBuffers(1, &ssbo_sky);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_sky);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sky), &sky, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_sky);
	}
	
	void setupChunksSSBO() {
		glGenBuffers(1, &ssbo_chunks);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_chunks);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Chunk) * world.chunks.size(), world.chunks.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_chunks);
	}
	
	void setupTrianglesSSBO() {
		glGenBuffers(1, &ssbo_triangles);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangles);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Triangle) * scene.triangles.size(), scene.triangles.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_triangles);
	}
	
	void setupBVHSSBO() {
		glGenBuffers(1, &ssbo_bvh_nodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_bvh_nodes);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH_Node) * scene.bvh_nodes.size(), scene.bvh_nodes.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_bvh_nodes);
	}

	static void errorCallback(int error, const char* description) {
		fprintf(stderr, "GLFW Error: %s\n", description);
		exit(EXIT_FAILURE);
	}

	static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
		if (engine != nullptr) {
			engine->keyCallback(window, key, scancode, action, mods);
		}
	}

	static void framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height) {
		Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
		if (engine != nullptr) {
			engine->framebufferSizeCallback(window, width, height);
		}
	}
	
	static void windowFocusCallbackStatic(GLFWwindow* window, int focused) {
		Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
		if (engine != nullptr) {
			engine->windowFocusCallback(window, focused);
		}
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_R) {
				if (is_rendering) {
					is_rendering = false;
					frame = 1;
				}
				else {
					is_rendering = true;
				}
			}
			else if (key == GLFW_KEY_ESCAPE) {
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
			else if (key == GLFW_KEY_PRINT_SCREEN) {
				int width, height;
				glfwGetFramebufferSize(window, &width, &height);

				GLubyte* pixels = new GLubyte[width * height * 3];
				glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

				std::time_t now = std::time(nullptr);
				char timestamp[20];
				std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
				std::string filename = "C:/GAMES/screenshot_" + std::string(timestamp) + ".png";

				stbi_flip_vertically_on_write(1);
				if (stbi_write_png(filename.c_str(), width, height, 3, pixels, 0)) {
					std::clog << "Successfully saved screenshot to: " << filename << std::endl;
				}
				else {
					std::clog << "Failed to save screenshot to: " << filename << std::endl;
				}

				delete[] pixels;
			}
			else if (key == GLFW_KEY_F11 && !is_rendering) {
				const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);

				if (glfwGetWindowMonitor(window) != nullptr) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

					glfwSetWindowMonitor(window, nullptr, 0, 0, video_mode->width / 4, video_mode->height / 4, GLFW_DONT_CARE);
					glfwSetWindowPos(window, 3 * video_mode->width / 8, 3 * video_mode->height / 8);
					glfwSetWindowSize(window, video_mode->width / 4, video_mode->height / 4);
				}
				else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

					glfwSetWindowMonitor(window, primary_monitor, 0, 0, video_mode->width, video_mode->height, GLFW_DONT_CARE);
				}
			}
		}
	}

	void processCameraMovement() {
		if (!is_rendering) {
			float speed = 0.5;
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_RELEASE) speed *= 3.146f;
			else if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_RELEASE) speed *= 0.318f;

			if (glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
				camera.stepForward(delta_time * speed);
			}
			if (glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
				camera.stepLeft(delta_time * speed);
			}
			if (glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
				camera.stepRight(delta_time * speed);
			}
			if (glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
				camera.stepBack(delta_time * speed);
			}
			if (glfwGetKey(window, GLFW_KEY_Q) != GLFW_RELEASE) {
				camera.stepDown(delta_time * speed);
			}
			if (glfwGetKey(window, GLFW_KEY_E) != GLFW_RELEASE) {
				camera.stepUp(delta_time * speed);
			}

			if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL) {
				double mouse_xpos, mouse_ypos;
				glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos);

				int width, height;
				glfwGetFramebufferSize(window, &width, &height);

				int centerX = width / 2;
				int centerY = height / 2;

				float x_velocity = (centerX - (float)mouse_xpos) / width * 45.0f;
				float y_velocity = (centerY - (float)mouse_ypos) / height * 45.0f;

				camera.pitch += y_velocity;
				camera.pitch = std::clamp(camera.pitch, -90.0f, 90.0f);
				camera.yaw += x_velocity;

				glfwSetCursorPos(window, centerX, centerY);
			}

			if (sky.type == SKY_TYPE_REALISTIC) {
				bool pass_new_value = false;

				if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_RELEASE) {
					sky = Sky(sky.pitch + delta_time * speed, sky.yaw);
					pass_new_value = true;
				}
				if (glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_RELEASE) {
					sky = Sky(sky.pitch - delta_time * speed, sky.yaw);
					pass_new_value = true;
				}
				if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE) {
					sky = Sky(sky.pitch, sky.yaw - delta_time * speed);
					pass_new_value = true;
				}
				if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE) {
					sky = Sky(sky.pitch, sky.yaw + delta_time * speed);
					pass_new_value = true;
				}

				if (pass_new_value) {
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_sky);
					glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Sky), &sky);
				}
			}
		}
	}

	void windowFocusCallback(GLFWwindow* window, int focused) {
		if (focused) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else {
			glfwRequestWindowAttention(window);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
		glUniform2f(resolution_location, static_cast<float>(width), static_cast<float>(height));

		frame = 1;

		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, normal_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Failed to resize framebuffer: " << status;
			exit(EXIT_FAILURE);
		}

		const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, draw_buffers);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLuint createShaderProgram(const std::string& vertex_shader_source, const std::string& fragment_shader_source) {
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		compileShader(vertex_shader, vertex_shader_source);

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(fragment_shader, fragment_shader_source);

		GLuint shader_program = glCreateProgram();
		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);
		glLinkProgram(shader_program);

		GLint success;
		GLchar info_log[512];
		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
		if (!success) {
			std::cerr << "Shader program linking failed: " << info_log;
			exit(EXIT_FAILURE);
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return shader_program;
	}

	void compileShader(GLuint shader, const std::string& shader_source) {
		const GLchar* shader_source_ptr = shader_source.c_str();
		glShaderSource(shader, 1, &shader_source_ptr, nullptr);
		glCompileShader(shader);

		GLint success;
		GLchar info_log[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 512, nullptr, info_log);
			std::cerr << "Shader compilation failed: " << std::string(info_log);
			exit(EXIT_FAILURE);
		}
	}

	template<typename FilePath>
	std::string readShaderFile(const FilePath& file_path) {
		std::ifstream file_stream(file_path, std::ios::in);
		if (!file_stream.is_open()) {
			std::cerr << "Could not read file " << std::string(file_path) << ". File does not exist.";
			exit(EXIT_FAILURE);
		}

		std::stringstream buffer;
		buffer << file_stream.rdbuf();
		file_stream.close();

		return buffer.str();
	}

	int frame = 1;
	float delta_time = 0.0;
	float time = 0.0;
	bool is_rendering = false;

	Mesh scene;
	BlockWorld world; 
	Denoiser pathtracing_denoiser;
	Camera camera;
	Camera old_camera;
	Sky sky = Sky(45.0, 60.0);

	GLuint ssbo_camera;
	GLuint ssbo_old_camera;
	GLuint ssbo_sky;
	GLuint ssbo_chunks;
	GLuint ssbo_triangles;
	GLuint ssbo_bvh_nodes;

	GLuint resolution_location;
	GLuint frame_location;
	GLuint frame_seed_location;
	GLuint time_location;
	GLuint delta_time_location;
	GLuint threads_working_location;

	GLuint map_texture;
	GLuint stone_texture;
	GLuint grass_top_texture;
	GLuint grass_side_texture;
	GLuint dirt_texture;

	GLuint diffuse_texture, // RGB - pathtraced color; A - number of accumulated colors
		albedo_texture,     // RGB - albedo
		normal_texture,     // RGB - normal
		position_texture;   // RGB - world position;   A - Depth

	GLuint framebuffer;

	GLuint main_program;
	GLuint postprocess_program;

	GLFWwindow* window;
	GLFWmonitor* primary_monitor;
};