#include "Scene.h"

#include <algorithm>

#define GLFW_EXPOSE_NATIVE_WIN32

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "SSBOmanager.h"
#include "Console.h"
#include "Timing.h"
#include "Window.h"
#include "BVH.h"

Scene Scene::Instance;

void Scene::pushLoadDialog() {
    wchar_t file_path[MAX_PATH] = {0};
    HWND hwnd = glfwGetWin32Window(Window::Instance.getWindow());

    WCHAR current_dir[MAX_PATH];
    if (!GetCurrentDirectory(MAX_PATH, current_dir)) {
        Console::Instance.push("Error getting current directory");
        return;
    }

    WCHAR initial_dir[MAX_PATH];
    wcscpy_s(initial_dir, current_dir);
    wcscat_s(initial_dir, L"\\Meshes");

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lpstrTitle = L"Select OBJ file";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file_path;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"OBJ Files\0*.obj\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"obj";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrInitialDir = initial_dir;

    if (!GetOpenFileNameW(&ofn)) {
        std::string err = std::to_string(CommDlgExtendedError());
        if (err != "0") Console::Instance.push("Failed to open file dialog: " + err);

        return;
    }

    if (!SetCurrentDirectory(current_dir)) {
        Console::Instance.push("Error setting current directory");
        return;
    }

    char utf8FilePath[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, file_path, -1, utf8FilePath, MAX_PATH, NULL, NULL);
    std::string file_path_str(utf8FilePath);

    Window::Instance.restore();
    loadOBJ(file_path_str);
    Window::Instance.restore();
}

void Scene::loadOBJ(const std::string& filepath) {
    size_t slash_index = filepath.find_last_of('/');
    if(slash_index == std::string::npos) slash_index = filepath.find_last_of('\\');

    std::string path = filepath.substr(0, slash_index + 1);

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;

    reader_config.triangulate = true;
    reader_config.triangulation_method = "earcut";
    reader_config.mtl_search_path = path;
    
    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            Console::Instance.push(reader.Error());
        }
        else {
            Console::Instance.push("Failed to load mesh from " + filepath);
        }

        return;
    }

    if (!reader.Warning().empty()) {
        Console::Instance.push(reader.Warning());
        return;
    }

    BVH::Instance.clear();
    m_meshes.clear();
    m_triangles.clear();
    m_materials.clear();
    m_light_ids.clear();

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

    size_t number_of_materials = materials.size();
    m_materials.resize(number_of_materials);

	// Loading materials
    for (size_t i = 0; i < number_of_materials; i++) {
        Material& material = m_materials[i];
        auto& mat = materials[i];

        material.name = mat.name;
        material.diffuse_color = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        material.emmision_color = glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
		material.roughness = mat.roughness;
        
        std::string diffuse_texname = mat.diffuse_texname;
        std::string alpha_texname = mat.alpha_texname;
        std::string emissive_texname = mat.emissive_texname;
        std::string normal_texname = (mat.normal_texname.empty() ? (mat.displacement_texname.empty() ? mat.bump_texname : mat.displacement_texname) : mat.normal_texname);
        std::string roughness_texname = mat.roughness_texname;

        if (!diffuse_texname.empty()) {
            Console::Instance.push("Loading diffuse texture: " + diffuse_texname);
            if (material.diffuse_texture.initFromFile(path + diffuse_texname)) {
                material.diffuse_tex_handle = material.diffuse_texture.bindAndGet();

                auto options = mat.diffuse_texopt;
                material.diffuse_offset = vec3(options.origin_offset[0], options.origin_offset[1], options.origin_offset[2]);
                material.diffuse_scale = vec3(options.scale[0], options.scale[1], options.scale[2]);

                material.use_diffuse_tex = true;
            }
        }
        
		if (!emissive_texname.empty()) {
            Console::Instance.push("Loading emission texture: " + emissive_texname);
            if (material.emissive_texture.initFromFile(path + emissive_texname)) {
                material.emissive_tex_handle = material.emissive_texture.bindAndGet();

                auto options = mat.emissive_texopt;
                material.emissive_offset = vec3(options.origin_offset[0], options.origin_offset[1], options.origin_offset[2]);
                material.emissive_scale = vec3(options.scale[0], options.scale[1], options.scale[2]);

                material.use_emissive_tex = true;
            }
		}

        if (!normal_texname.empty()) {
            Console::Instance.push("Loading normal texture: " + normal_texname);
            if (material.normal_texture.initFromFile(path + normal_texname)) {
                material.normal_tex_handle = material.normal_texture.bindAndGet();

                auto options = mat.normal_texopt;
                material.normal_offset = vec3(options.origin_offset[0], options.origin_offset[1], options.origin_offset[2]);
                material.normal_scale = vec3(options.scale[0], options.scale[1], options.scale[2]);

                material.use_normal_tex = true;
            }
        }

        if (!roughness_texname.empty()) {
            Console::Instance.push("Loading roughness texture: " + roughness_texname);
            if (material.roughness_texture.initFromFile(path + roughness_texname)) {
                material.roughness_tex_handle = material.roughness_texture.bindAndGet();

                auto options = mat.roughness_texopt;
                material.roughness_offset = vec3(options.origin_offset[0], options.origin_offset[1], options.origin_offset[2]);
                material.roughness_scale = vec3(options.scale[0], options.scale[1], options.scale[2]);

                material.use_roughness_tex = true;
            }
        }
    }

	// Loading triangles
    for (const auto& shape : shapes) {
        const auto& mesh = shape.mesh;
        const auto& mat = mesh.material_ids;

        for (size_t face_index = 0; face_index < mesh.indices.size() / 3; face_index++) {
            tinyobj::index_t idx0 = mesh.indices[face_index * 3 + 0];
            tinyobj::index_t idx1 = mesh.indices[face_index * 3 + 1];
            tinyobj::index_t idx2 = mesh.indices[face_index * 3 + 2];

            Triangle triangle;

            triangle.v0.x = attrib.vertices[3 * idx0.vertex_index + 0];
            triangle.v0.y = attrib.vertices[3 * idx0.vertex_index + 1];
            triangle.v0.z = attrib.vertices[3 * idx0.vertex_index + 2];
            triangle.v1.x = attrib.vertices[3 * idx1.vertex_index + 0];
            triangle.v1.y = attrib.vertices[3 * idx1.vertex_index + 1];
            triangle.v1.z = attrib.vertices[3 * idx1.vertex_index + 2];
            triangle.v2.x = attrib.vertices[3 * idx2.vertex_index + 0];
            triangle.v2.y = attrib.vertices[3 * idx2.vertex_index + 1];
            triangle.v2.z = attrib.vertices[3 * idx2.vertex_index + 2];

            if (idx0.normal_index == -1) {
                vec3 e1 = triangle.v1 - triangle.v0;
                vec3 e2 = triangle.v2 - triangle.v0;

                vec3 normal = normalize(cross(e1, e2));
                triangle.n0 = normal;
                triangle.n1 = normal;
                triangle.n2 = normal;
            }
            else {
                triangle.n0.x = attrib.normals[3 * idx0.normal_index + 0];
                triangle.n0.y = attrib.normals[3 * idx0.normal_index + 1];
                triangle.n0.z = attrib.normals[3 * idx0.normal_index + 2];
                triangle.n1.x = attrib.normals[3 * idx1.normal_index + 0];
                triangle.n1.y = attrib.normals[3 * idx1.normal_index + 1];
                triangle.n1.z = attrib.normals[3 * idx1.normal_index + 2];
                triangle.n2.x = attrib.normals[3 * idx2.normal_index + 0];
                triangle.n2.y = attrib.normals[3 * idx2.normal_index + 1];
                triangle.n2.z = attrib.normals[3 * idx2.normal_index + 2];
            }

            if(idx0.texcoord_index != -1) {
                triangle.uv0.x = attrib.texcoords[2 * idx0.texcoord_index + 0];
                triangle.uv0.y = attrib.texcoords[2 * idx0.texcoord_index + 1];
                triangle.uv1.x = attrib.texcoords[2 * idx1.texcoord_index + 0];
                triangle.uv1.y = attrib.texcoords[2 * idx1.texcoord_index + 1];
                triangle.uv2.x = attrib.texcoords[2 * idx2.texcoord_index + 0];
                triangle.uv2.y = attrib.texcoords[2 * idx2.texcoord_index + 1];
            }

            triangle.material_id = static_cast<int>(mat[face_index]);
            
            Material& material = m_materials[triangle.material_id];
            if (material.use_emissive_tex || any(notEqual(material.emmision_color, vec3(0.0f)))) {
                int light_index = static_cast<int>(m_triangles.size());
                m_light_ids.push_back(light_index);
            }

            m_triangles.push_back(triangle);
        }

        if (!m_combine_meshes) {
            Mesh umesh;
            umesh.name = shape.name;
            umesh.last_triangle_index = m_triangles.size() - 1;
            m_meshes.push_back(umesh);
        }
    }

    if (m_combine_meshes) {
        Mesh umesh;
        umesh.name = "CombinedMesh" + m_meshes.size();
		umesh.last_triangle_index = m_triangles.size() - 1;
		m_meshes.push_back(umesh);
    }

    // Pass meshes and triangles to the GPU
    if (!m_triangles.empty() && !m_meshes.empty()) {
        BVH::Instance.setCurrentMeshFilepath(filepath);

        Timing timing;
        timing.start();

        int start = 0;
        for (Mesh& mesh : m_meshes) {
            int end = mesh.last_triangle_index + 1;
            BVH::Instance.buildBLAS(mesh, m_triangles, start, end);
            start = end;
        }

        timing.stop();
        Console::Instance.push("BLAS build time: " + std::to_string(timing.getElapsed()) + " ms");

        timing.start();
        BVH::Instance.buildTLAS(m_meshes);
        timing.stop();

        Console::Instance.push("TLAS build time: " + std::to_string(timing.getElapsed()) + " ms");
        //std::cout << "BVH data: " << double(m_nodes.size() * sizeof(BVH_Node)) / 1048576.0 << " MB" << std::endl;

        BVH::Instance.transferToGPU();

        m_info.number_of_triangles = m_triangles.size();
        m_info.number_of_meshes = m_meshes.size();
        
        size_t tris_size = m_info.number_of_triangles * sizeof(Triangle);
        size_t meshes_size = m_info.number_of_meshes * sizeof(UniformMesh);

        UniformMesh* umeshes = new UniformMesh[m_info.number_of_meshes];
        
        std::transform(m_meshes.begin(), m_meshes.end(), umeshes,
            [](const Mesh& mesh) { return static_cast<UniformMesh>(mesh); }
        );

        SSBOmanager::Instance.createOrSet(SSBO_TRIANGLES_BINDING, m_triangles.data(), tris_size);
        SSBOmanager::Instance.createOrSet(SSBO_MESHES_BINDING, umeshes, meshes_size);
        delete[] umeshes;

        Console::Instance.push("Triangles data: " + std::to_string(double(tris_size) / 1048576.0) + " MB");
        Console::Instance.push("Number of meshes: " + std::to_string(m_info.number_of_meshes));
    }

	// Pass lights to the GPU
    if (!m_light_ids.empty()) {
        m_info.number_of_lights = m_light_ids.size();
		size_t lights_size = m_info.number_of_lights * sizeof(int);
        SSBOmanager::Instance.createOrSet(SSBO_LIGHTS_BINDING, m_light_ids.data(), lights_size);

        Console::Instance.push("Lights count: " + std::to_string(m_info.number_of_lights));
    }
    else {
        SSBOmanager::Instance.createOrSet(SSBO_LIGHTS_BINDING, nullptr, 0);
        m_info.number_of_lights = 0;
    }

	// Pass materials to the GPU
    if (!m_materials.empty()) {
        m_info.number_of_materials = m_materials.size();
        size_t mats_size = m_info.number_of_materials * sizeof(UniformMaterial);

        UniformMaterial* umaterials = new UniformMaterial[m_info.number_of_materials];

        std::transform(m_materials.begin(), m_materials.end(), umaterials, 
            [](const Material& material) { return static_cast<UniformMaterial>(material); }
        );

        SSBOmanager::Instance.createOrSet(SSBO_MATERIALS_BINDING, umaterials, mats_size);
        delete[] umaterials;
    }
}

void Scene::setCombineMeshes(bool value) {
    m_combine_meshes = value;
}

const std::vector<Material>& Scene::getMaterials() {
    return m_materials;
}

const std::vector<Mesh>& Scene::getMeshes() {
    return m_meshes;
}

const Mesh& Scene::getMesh(int index) {
    return m_meshes[index];
}

const Mesh& Scene::getMesh(const std::string& name) {
    for (const Mesh& mesh : m_meshes) {
        if (mesh.name == name) return mesh;
    }
}

Material* Scene::getMaterial(int index) {
    if (index >= m_materials.size()) {
        Console::Instance.push("Attempt to get material by non-existent index");
        return nullptr;
    }

    return &m_materials[index];
}

void Scene::updateMaterial(int index) {
    Buffer* mbuf = SSBOmanager::Instance.get(SSBO_MATERIALS_BINDING);
    mbuf->update((UniformMaterial*)&m_materials[index], index * sizeof(UniformMaterial), sizeof(UniformMaterial));
}

const SceneInfo& Scene::getInfo() {
    return m_info;
}