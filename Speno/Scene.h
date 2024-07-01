#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "Mesh.h"
#include "Triangle.h"
#include "Material.h"

struct SceneInfo {
	size_t number_of_meshes;
	size_t number_of_triangles;
	size_t number_of_lights;
	size_t number_of_materials;
};

class Scene {
public:
	static Scene Instance;

	void pushLoadDialog();
	void loadOBJ(const std::string& filepath);
	void setCombineMeshes(bool value);

	const Mesh& getMesh(int index);
	const Mesh& getMesh(const std::string& name);
	Material* getMaterial(int index);
	void updateMaterial(int index);

	const std::vector<Material>& getMaterials();
	const std::vector<Mesh>& getMeshes();
	const SceneInfo& getInfo();

	Scene(Scene const&) = delete;
	void operator=(Scene const&) = delete;

private:
	Scene() {}

	SceneInfo m_info;
	std::vector<Mesh> m_meshes;
	std::vector<Triangle> m_triangles;
	std::vector<int> m_light_ids;
	std::vector<Material> m_materials;

	bool m_combine_meshes = true;
};