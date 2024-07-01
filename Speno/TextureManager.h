#pragma once

#include <map>
#include <memory>

#include <glad/glad.h>

#include "Texture.h"

class TextureManager {
public:
	static TextureManager Instance;

	TextureManager(TextureManager const&) = delete;
	void operator=(TextureManager const&) = delete;

	Texture* add(const std::string& name);
	Texture* get(const std::string& name);

	void setupSTBN();
	void bindSTBN(std::string program_name);

private:
	TextureManager() {}

private:
	std::map<std::string, std::shared_ptr<Texture>> m_textures;
};