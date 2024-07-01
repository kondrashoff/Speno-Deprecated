#include "Atmosphere.h"

#include "UBOmanager.h"

Atmosphere Atmosphere::Instance;

void Atmosphere::init() {
	m_atmosphere_program.initPixelShader("atmosphere.frag");
	m_atmosphere_program.setResolution(4096, 2048);

	TextureParameters tp;
	tp.min_filter = GL_LINEAR;
	tp.mag_filter = GL_LINEAR;

	Framebuffer* fbo = m_atmosphere_program.getFBO();
	fbo->init();
	fbo->addAttachment("rendered_atmosphere", GL_RGB32F, tp);
	fbo->resize(4096, 2048, 1.0f);
	m_rendered_atmosphere = fbo->getAttachment("rendered_atmosphere");
	
	buildAtmosphereDirection();
	update();

	UBOmanager& um = UBOmanager::Instance;
	um.create("atmosphereData", &m_uniform_atmosphere, sizeof(UniformAtmosphere), GL_DYNAMIC_DRAW);
	um.bind("atmosphereData", m_atmosphere_program.getID());
}

void Atmosphere::render() {
	if (m_should_render) {
		m_atmosphere_program.draw();
		m_should_render = false;
	}

	update();
}

void Atmosphere::update() {
	if (m_rendered_pitch != m_pitch || m_rendered_yaw != m_yaw || m_rendered_roll != m_roll) {
		m_rendered_pitch = m_pitch;
		m_rendered_yaw = m_yaw;
		m_rendered_roll = m_roll;

		buildAtmosphereDirection();
		m_should_render = true;
	}

	if (m_uniform_atmosphere.quality_i != m_quality_i || m_uniform_atmosphere.quality_j != m_quality_j) {
		m_uniform_atmosphere.quality_i = m_quality_i;
		m_uniform_atmosphere.quality_j = m_quality_j;
		m_should_render = true;
	}
}

void Atmosphere::buildAtmosphereDirection() {
	float pr = radians(m_pitch);
	float sx = sin(pr);
	float cx = cos(pr);

	float yr = radians(m_yaw);
	float sy = sin(yr);
	float cy = cos(yr);

	float rr = radians(m_roll);
	float sz = sin(rr);
	float cz = cos(rr);

	mat3 rotation = mat3(cy * cz, -cy * sz, sy, sx * sy * cz + cx * sz, -sx * sy * sz + cx * cz, -sx * cy, -cx * sy * cz + sx * sz, cx * sy * sz + sx * cz, cx * cy);
	m_uniform_atmosphere.direction = rotation * vec3(0.0f, 0.0f, 1.0f);
}

void Atmosphere::bind(std::string program_name) {
	UBOmanager::Instance.bind("atmosphereData", program_name);
}

Texture* Atmosphere::getTexture() {
	return m_rendered_atmosphere;
}