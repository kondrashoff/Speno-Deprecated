#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

#include "Quad.h"
#include "Framebuffer.h"

class Program {
public:
	void draw();
	void initPixelShader(const std::string& fragment_shader_filename);
	void initCompute(const std::string& shader_filename);
	bool initBinary(const std::string& shader_filename);

	void setResolution(int width, int height);
	void setResolutionScale(float scale);
	void resizeFBO(int width, int height);

	float getResolutionScale();
	Framebuffer* getFBO();
	GLuint getID();

	void enable();
	void disable();

	~Program();

private:
	std::string readShaderFile(const std::string& filename);
	bool compileShader(GLuint shader, const std::string& shader_source);

private:
	GLuint m_id;
	bool m_is_disabled = true;
	bool m_is_initialized = false;

	Framebuffer m_framebuffer;
	float m_resolution_scale = 1.0f;
	bool m_use_custom_resolution = false;
	int m_width, m_height;

	std::vector<std::string> m_filename_at_line;
};