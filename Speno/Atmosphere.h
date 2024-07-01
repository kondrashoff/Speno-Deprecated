#pragma once

#include <glm/glm.hpp>
using namespace glm;

#include "Program.h"

struct UniformAtmosphere {
	alignas(16) vec3 direction;
	int quality_i;
	int quality_j;
};

class Atmosphere {
public:
	static Atmosphere Instance;

	void init();
	void render();
	void bind(std::string program_name);
	Texture* getTexture();

	Atmosphere(Atmosphere const&) = delete;
	void operator=(Atmosphere const&) = delete;

private:
	Atmosphere() {}

	void update();
	void buildAtmosphereDirection();

public:
	float m_pitch = 30.0f;
	float m_yaw = 60.0f;
	float m_roll = 0.0f;

	// The most ideal values in most cases
	int m_quality_i = 32;
	int m_quality_j = 16;

private:
	Program m_atmosphere_program;
	UniformAtmosphere m_uniform_atmosphere;
	Texture* m_rendered_atmosphere;

	bool m_should_render = true;

	float m_rendered_pitch = m_pitch;
	float m_rendered_yaw = m_yaw;
	float m_rendered_roll = m_roll;
};