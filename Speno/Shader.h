#pragma once

glm::vec3 sphericalMap(glm::vec2 uv) {
	uv -= 0.5; uv *= glm::vec2(6.2831853071, 3.1415926535);
	glm::vec2 s = glm::sin(uv), c = glm::cos(uv);
	return glm::vec3(c.x * c.y, s.y, s.x * c.y);
}

float computeLuminance(glm::vec3 color) {
	return glm::dot(color, glm::vec3(0.2126, 0.7152, 0.0722));
}

class ShaderProgram {
public:

	ShaderProgram() = default;

	void init(std::string shader_filename) {
		std::string vertex_shader_source = readShaderFile("vertex.glsl");
		std::string fragment_shader_source = readShaderFile(shader_filename);

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
			std::cout << "Shader program linking failed: " << info_log;
			exit(EXIT_FAILURE);
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		id = shader_program;
	}

	void createFBO(int width, int height, int format, int filter, int wrap) {
		if (use_gbuffers) {
			std::cout << "FBO is already created.";
			exit(EXIT_FAILURE);
		}

		glUseProgram(id);
		use_fbo = true;

		fbo_format = format;
		fbo_filter = filter;
		fbo_wrap = wrap;

		glGenTextures(1, &fbo_texture);
		glBindTexture(GL_TEXTURE_2D, fbo_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Failed to create framebuffer: " << status;
			exit(EXIT_FAILURE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glUseProgram(0);
	}

	void createGBuffers(int width, int height, int format, int filter, int wrap) {
		if (use_fbo) {
			std::cout << "FBO is already created.";
			exit(EXIT_FAILURE);
		}

		glUseProgram(id);
		use_gbuffers = true;
		use_fbo = true;

		fbo_format = format;
		fbo_filter = filter;
		fbo_wrap = wrap;

		glGenTextures(1, &fbo_texture);
		glBindTexture(GL_TEXTURE_2D, fbo_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &gbuffer_albedo_texture);
		glBindTexture(GL_TEXTURE_2D, gbuffer_albedo_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &gbuffer_normal_texture);
		glBindTexture(GL_TEXTURE_2D, gbuffer_normal_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &gbuffer_position_texture);
		glBindTexture(GL_TEXTURE_2D, gbuffer_position_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbuffer_albedo_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer_normal_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gbuffer_position_texture, 0);

		GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, draw_buffers);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Failed to create framebuffer: " << status;
			exit(EXIT_FAILURE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glUseProgram(0);
	}

	void resize(int width, int height) {
		if (use_fbo) {
			glUseProgram(id);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glBindTexture(GL_TEXTURE_2D, fbo_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, fbo_format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

			if (use_gbuffers) {
				glBindTexture(GL_TEXTURE_2D, gbuffer_albedo_texture);
				glTexImage2D(GL_TEXTURE_2D, 0, fbo_format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

				glBindTexture(GL_TEXTURE_2D, gbuffer_normal_texture);
				glTexImage2D(GL_TEXTURE_2D, 0, fbo_format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

				glBindTexture(GL_TEXTURE_2D, gbuffer_position_texture);
				glTexImage2D(GL_TEXTURE_2D, 0, fbo_format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbuffer_albedo_texture, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer_normal_texture, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gbuffer_position_texture, 0);
			}
			
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Failed to resize framebuffer: " << status;
				exit(EXIT_FAILURE);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glUseProgram(0);
		}
	}

	void bindTexture(GLuint texture_unit, GLuint texture, std::string uniform_name, int texture_type = GL_TEXTURE_2D) {
		glUseProgram(id);
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(texture_type, texture);
		glUniform1i(glGetUniformLocation(id, uniform_name.c_str()), texture_unit);
		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

	void textureLoadFromFile(GLuint texture_unit, std::string filepath, std::string uniform_name, int filter, int wrap) {
		glUseProgram(id);
		glActiveTexture(GL_TEXTURE0 + texture_unit);

		global_prefomance_monitor.start();

		int width, height, nrChannels;
		float* data = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);

		if (data) {
			global_prefomance_monitor.stop();
			console_buffer += "Texture load time: " + std::to_string(global_prefomance_monitor.ms) + " ms\n";

			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);

			if (nrChannels == 3) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
			}
			else if (nrChannels == 4) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
			}

			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);

			bindTexture(texture_unit, texture, uniform_name);

			stbi_image_free(data);
		}
		else {
			console_buffer += "Failed to load texture: " + std::string(filepath) + "\n";
			stbi_image_free(data);
		}

		glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
	}

	void hdriLoadFromFile(GLuint texture_unit, std::string filepath, std::string uniform_name, GLuint data_texture_unit, std::string data_uniform_name) {
		glUseProgram(id);
		global_prefomance_monitor.start();

		int width, height, nrChannels;
		float* data = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);
		
		if (!data) {
			console_buffer += "Failed to load texture: " + std::string(filepath) + "\n";
			stbi_image_free(data);
			glUseProgram(0);
			return;
		}

		global_prefomance_monitor.stop();
		console_buffer += "Hdri loaded in " + std::to_string(global_prefomance_monitor.ms) + " ms\n";

		glUseProgram(id);
		
		GLuint texture;
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glUniform1i(glGetUniformLocation(id, uniform_name.c_str()), texture_unit);
		glActiveTexture(GL_TEXTURE0);

		global_prefomance_monitor.start();

		std::vector<glm::vec4> luminance_array;
		float maximum_luminance = 0.0f;

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				int coord = x + y * width;
				float luminance = computeLuminance(glm::vec3(data[coord * 3], data[coord * 3 + 1], data[coord * 3 + 2]));
				shared_engine_data.hdri_luminance_sum += luminance;
				glm::vec3 direction = sphericalMap(glm::vec2(x, y) / glm::vec2(width, height));
				luminance_array.push_back(glm::vec4(direction, luminance));

				if (luminance > maximum_luminance) {
					maximum_luminance = luminance;
					shared_engine_data.sun_direction = direction;
				}
			}
		}

		std::sort(luminance_array.begin(), luminance_array.end(), [](glm::vec4 a, glm::vec4 b) { return a.w > b.w; });
			
		float* luminance_data = new float[luminance_array.size() * 4];
		for (int i = 0; i < luminance_array.size(); i++) {
			luminance_data[i * 4] = 0.5f + 0.5f * luminance_array[i].x;
			luminance_data[i * 4 + 1] = 0.5f + 0.5f * luminance_array[i].y;
			luminance_data[i * 4 + 2] = 0.5f + 0.5f * luminance_array[i].z;
			luminance_data[i * 4 + 3] = luminance_array[i].w;
		}

		GLuint luminance_texture;
		glGenTextures(1, &luminance_texture);
		glActiveTexture(GL_TEXTURE0 + data_texture_unit);
		glBindTexture(GL_TEXTURE_2D, luminance_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, luminance_data);
		glUniform1i(glGetUniformLocation(id, data_uniform_name.c_str()), data_texture_unit);
		glActiveTexture(GL_TEXTURE0);

		global_prefomance_monitor.stop();
		console_buffer += "Hdri data generated in " + std::to_string(global_prefomance_monitor.ms) + " ms\n";

		delete[] luminance_data;
		luminance_array.clear();
		stbi_image_free(data);
		glUseProgram(0);
	}

	inline void use() {
		glUseProgram(id);
		
		if(use_fbo) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		}
	}

	inline void unuse() {
		if (use_fbo) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		glUseProgram(0);
	}

	void setUniform1f(std::string uniform_name, float value) {
		glUseProgram(id);
		glUniform1f(glGetUniformLocation(id, uniform_name.c_str()), value);
		glUseProgram(0);
	}

	void setUniform1i(std::string uniform_name, int value) {
		glUseProgram(id);
		glUniform1i(glGetUniformLocation(id, uniform_name.c_str()), value);
		glUseProgram(0);
	}

	void setUniform1ui(std::string uniform_name, unsigned int value) {
		glUseProgram(id);
		glUniform1ui(glGetUniformLocation(id, uniform_name.c_str()), value);
		glUseProgram(0);
	}

	void setUniform1b(std::string uniform_name, bool value) {
		glUseProgram(id);
		glUniform1i(glGetUniformLocation(id, uniform_name.c_str()), value);
		glUseProgram(0);
	}

	void setUniform2f(std::string uniform_name, glm::vec2 value) {
		glUseProgram(id);
		glUniform2f(glGetUniformLocation(id, uniform_name.c_str()), value.x, value.y);
		glUseProgram(0);
	}

	void setUniform3f(std::string uniform_name, glm::vec3 value) {
		glUseProgram(id);
		glUniform3f(glGetUniformLocation(id, uniform_name.c_str()), value.x, value.y, value.z);
		glUseProgram(0);
	}

	void setUniform3i(std::string uniform_name, glm::ivec3 value) {
		glUseProgram(id);
		glUniform3i(glGetUniformLocation(id, uniform_name.c_str()), value.x, value.y, value.z);
		glUseProgram(0);
	}

	~ShaderProgram() {
		glDeleteTextures(1, &fbo_texture);
		glDeleteFramebuffers(1, &fbo);
		glDeleteRenderbuffers(1, &rbo);
		glDeleteProgram(id);
	}

private:

	std::string readShaderFile(std::string filename) {
		std::string filepath = "Shader/";
		std::ifstream file_stream(filepath + filename, std::ios::in);

		if (!file_stream.is_open()) {
			std::cout << "Could not read file " << std::string(filename) << ". File does not exist.";
			exit(EXIT_FAILURE);
		}

		std::stringstream buffer;
		buffer << file_stream.rdbuf();
		file_stream.close();

		std::string shader_source = buffer.str();

		std::regex include_pattern("#include \"([^\"]+)\"");
		std::smatch include_matches;

		while (std::regex_search(shader_source, include_matches, include_pattern)) {
			std::string include_directive = include_matches[0];
			std::string include_file_name = include_matches[1];

			std::string include_source = readShaderFile(include_file_name);

			shader_source.replace(include_matches.position(0), include_directive.length(), include_source + "\n");
		}

		return shader_source;
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
			std::string s_info_log = std::string(info_log);
			std::cout << "Shader compilation failed: " << s_info_log << std::endl;

			std::regex line_number_regex("ERROR: [0-9]+:([0-9]+)");
			std::smatch matches;
			if (std::regex_search(s_info_log, matches, line_number_regex)) {
				int error_line_number = std::stoi(matches[1]);

				// Print line of code that caused the error
				std::stringstream ss(shader_source);
				std::string line;
				int current_line_number = 1;
				while (std::getline(ss, line)) {
					if (current_line_number++ == error_line_number) {
						std::cout << "Error in line " << error_line_number << ":\n" << line << std::endl;
						break;
					}
				}
			}

			exit(EXIT_FAILURE);
		}
	}
	
public:
	
	GLuint id;
	GLuint fbo;
	GLuint fbo_texture;
	GLuint gbuffer_albedo_texture;
	GLuint gbuffer_normal_texture;
	GLuint gbuffer_position_texture;
	GLuint rbo;

private:

	int fbo_format, fbo_filter, fbo_wrap;
	bool use_gbuffers = false;
	bool use_fbo = false;
};