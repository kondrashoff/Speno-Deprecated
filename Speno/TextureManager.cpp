#include "TextureManager.h"

#include <iostream>

TextureManager TextureManager::Instance;

void TextureManager::setupSTBN() {
	TextureParameters params;
	params.wrap_s = GL_REPEAT;
	params.wrap_t = GL_REPEAT;
	params.min_filter = GL_NEAREST;
	params.mag_filter = GL_NEAREST;

	int width = 128;
	int height = 128;
	int layers = 128;

	Texture* scalar = add("stbn_scalar");
	Texture* vec1 = add("stbn_vec1");
	Texture* vec2 = add("stbn_vec2");
	Texture* vec3 = add("stbn_vec3");
	Texture* vec1_unit = add("stbn_unitvec1");
	Texture* vec2_unit = add("stbn_unitvec2");
	Texture* vec3_unit = add("stbn_unitvec3");
	Texture* vec3_unit_cosine = add("stbn_unitvec3_cosine");

	std::string scalar_files = "STBN/scalar/stbn_scalar_";
	std::string vec1_files = "STBN/vec1/stbn_vec1_";
	std::string vec2_files = "STBN/vec2/stbn_vec2_";
	std::string vec3_files = "STBN/vec3/stbn_vec3_";
	std::string vec1_unit_files = "STBN/vec1/unit/stbn_unitvec1_";
	std::string vec2_unit_files = "STBN/vec2/unit/stbn_unitvec2_";
	std::string vec3_unit_files = "STBN/vec3/unit/stbn_unitvec3_";
	std::string vec3_unit_cosine_files = "STBN/vec3/unit/cosine/stbn_unitvec3_cosine_";

	scalar->initLayered(width, height, layers, params);
	vec1->initLayered(width, height, layers, params);
	vec2->initLayered(width, height, layers, params);
	vec3->initLayered(width, height, layers, params);
	vec1_unit->initLayered(width, height, layers, params);
	vec2_unit->initLayered(width, height, layers, params);
	vec3_unit->initLayered(width, height, layers, params);
	vec3_unit_cosine->initLayered(width, height, layers, params);

	for (int i = 0; i < 128; i++) {
		std::string scalar_filename = scalar_files + std::to_string(i) + ".png";
		std::string vec1_filename = vec1_files + std::to_string(i) + ".png";
		std::string vec2_filename = vec2_files + std::to_string(i) + ".png";
		std::string vec3_filename = vec3_files + std::to_string(i) + ".png";
		std::string vec1_unit_filename = vec1_unit_files + std::to_string(i) + ".png";
		std::string vec2_unit_filename = vec2_unit_files + std::to_string(i) + ".png";
		std::string vec3_unit_filename = vec3_unit_files + std::to_string(i) + ".png";
		std::string vec3_unit_cosine_filename = vec3_unit_cosine_files + std::to_string(i) + ".png";

		scalar->addLayerFromFile(scalar_filename);
		vec1->addLayerFromFile(vec1_filename);
		vec2->addLayerFromFile(vec2_filename);
		vec3->addLayerFromFile(vec3_filename);
		vec1_unit->addLayerFromFile(vec1_unit_filename);
		vec2_unit->addLayerFromFile(vec2_unit_filename);
		vec3_unit->addLayerFromFile(vec3_unit_filename);
		vec3_unit_cosine->addLayerFromFile(vec3_unit_cosine_filename);
	}
}

void TextureManager::bindSTBN(std::string program_name) {
	get("stbn_scalar")->bind("stbn_scalar", program_name);
	get("stbn_vec1")->bind("stbn_vec1", program_name);
	get("stbn_vec2")->bind("stbn_vec2", program_name);
	get("stbn_vec3")->bind("stbn_vec3", program_name);
	get("stbn_unitvec1")->bind("stbn_unitvec1", program_name);
	get("stbn_unitvec2")->bind("stbn_unitvec2", program_name);
	get("stbn_unitvec3")->bind("stbn_unitvec3", program_name);
	get("stbn_unitvec3_cosine")->bind("stbn_unitvec3_cosine", program_name);
}

Texture* TextureManager::add(const std::string& name) {
	m_textures.emplace(name, std::make_unique<Texture>());
	return m_textures[name].get();
}

Texture* TextureManager::get(const std::string& name) {
	auto it = m_textures.find(name);

	if (it == m_textures.end()) 
		throw std::runtime_error("Texture " + name + " not found");

	return it->second.get();
}