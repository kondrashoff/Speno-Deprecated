#pragma once

#include <memory>
#include <map>

#include "Buffer.h"

constexpr int SSBO_TRIANGLES_BINDING = 0;
constexpr int SSBO_MATERIALS_BINDING = 1;
constexpr int SSBO_BVH_BINDING = 2;
constexpr int SSBO_LIGHTS_BINDING = 3;
constexpr int SSBO_MESHES_BINDING = 4;

class SSBOmanager {
public:
	static SSBOmanager Instance;

	Buffer* create(int binding, void* data, size_t size, GLenum usage = GL_STATIC_DRAW);
	Buffer* createOrSet(int binding, void* data, size_t size, GLenum usage = GL_STATIC_DRAW);
	Buffer* add(int binding);
	Buffer* get(int binding);
	void update();

private:
	std::map<int, std::unique_ptr<Buffer>> m_ssbos;
};