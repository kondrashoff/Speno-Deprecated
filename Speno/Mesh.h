#pragma once

struct AABB {
    alignas(16) glm::vec3 minimum;
    alignas(16) glm::vec3 maximum;

    inline float surfaceArea() {
		glm::vec3 d = maximum - minimum;
		return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
    }
};

inline AABB mergeAABB(const AABB& a, const AABB& b) {
    glm::vec3 min = glm::min(a.minimum, b.minimum);
    glm::vec3 max = glm::max(a.maximum, b.maximum);
    return AABB(min, max);
}

inline AABB subtractAABB(const AABB& a, const AABB& b) {
    glm::vec3 min = glm::max(a.minimum, b.minimum);
	glm::vec3 max = glm::min(a.maximum, b.maximum);
	return AABB(min, max);
}

// Utility function to compute cost based on SAH
inline float computeCost(AABB& aabb, int numPrimitives) {
    // Assuming unit cost per intersection test
    return aabb.surfaceArea() * numPrimitives;
}

struct BVH_Node {
    AABB aabb;
    int left_index;
    int right_index;
};

struct Triangle {
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 normal;
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec2 uv2;
    glm::vec3 color;
    glm::vec3 center;
    AABB bounding_box;
};

struct UniformTriangle {
    alignas(16) glm::vec3 v0;
    alignas(16) glm::vec3 v1;
    alignas(16) glm::vec3 v2;
    alignas(16) glm::vec3 normal;
    alignas(8) glm::vec2 uv0;
    alignas(8) glm::vec2 uv1;
    alignas(8) glm::vec2 uv2;
    alignas(16) glm::vec3 color;
};

class Mesh {
public:

    Mesh() = default;
    
    ~Mesh() {
        glDeleteBuffers(1, &ssbo_triangles);
		glDeleteBuffers(1, &ssbo_bvh_nodes);
    }

    void loadFromFile(std::string filepath) {
        global_prefomance_monitor.start();

        std::string path = filepath.substr(0, filepath.find_last_of('/') + 1);
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = path; // Path to material files

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(filepath, reader_config)) {
            if (!reader.Error().empty()) {
                std::cout << "TinyObjReader error: " << reader.Error();
                console_buffer += reader.Error();
                return;
            }
        }

        if (already_loaded) {
            glDeleteBuffers(1, &ssbo_triangles);
            glDeleteBuffers(1, &ssbo_bvh_nodes);

            triangles.clear();
            bvh_nodes.clear();
            uniform_triangles.clear();
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader error: " << reader.Warning();
            console_buffer += reader.Warning();
        }

        const tinyobj::attrib_t& attrib = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
        const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

        for (const auto& shape : shapes) {
            const auto& mesh = shape.mesh;
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

                triangle.normal.x = attrib.normals[3 * idx0.normal_index + 0];
                triangle.normal.y = attrib.normals[3 * idx0.normal_index + 1];
                triangle.normal.z = attrib.normals[3 * idx0.normal_index + 2];

                triangle.uv0.x = attrib.texcoords[2 * idx0.texcoord_index + 0];
                triangle.uv0.y = attrib.texcoords[2 * idx0.texcoord_index + 1];
                triangle.uv1.x = attrib.texcoords[2 * idx1.texcoord_index + 0];
                triangle.uv1.y = attrib.texcoords[2 * idx1.texcoord_index + 1];
                triangle.uv2.x = attrib.texcoords[2 * idx2.texcoord_index + 0];
                triangle.uv2.y = attrib.texcoords[2 * idx2.texcoord_index + 1];

                triangle.color.x = attrib.colors[3 * idx0.vertex_index + 0];
                triangle.color.y = attrib.colors[3 * idx0.vertex_index + 1];
                triangle.color.z = attrib.colors[3 * idx0.vertex_index + 2];

                triangles.push_back(triangle);
            }
        }

        global_prefomance_monitor.stop();
        console_buffer += "Mesh loaded in " + std::to_string(global_prefomance_monitor.ms) + " ms\n";

        global_prefomance_monitor.start();
        buildBVH();
        global_prefomance_monitor.stop();
        console_buffer += "BVH built in " + std::to_string(global_prefomance_monitor.ms) + " ms\n";

        for (Triangle& triangle : triangles) {
            UniformTriangle uniform_triangle;
			uniform_triangle.v0 = triangle.v0;
			uniform_triangle.v1 = triangle.v1;
			uniform_triangle.v2 = triangle.v2;
            uniform_triangle.normal = triangle.normal;
			uniform_triangle.uv0 = triangle.uv0;
			uniform_triangle.uv1 = triangle.uv1;
			uniform_triangle.uv2 = triangle.uv2;
			uniform_triangle.color = triangle.color;
			uniform_triangles.emplace_back(uniform_triangle);
		}

        createSSBO();
        
        already_loaded = true;
    }

private:

    void createSSBO() {
        global_prefomance_monitor.start();

        glGenBuffers(1, &ssbo_triangles);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangles);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UniformTriangle) * uniform_triangles.size(), uniform_triangles.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_triangles);

        glGenBuffers(1, &ssbo_bvh_nodes);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_bvh_nodes);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH_Node) * bvh_nodes.size(), bvh_nodes.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_bvh_nodes);

        global_prefomance_monitor.stop();
        console_buffer += "Triangles and bvh nodes uploaded to GPU in " + std::to_string(global_prefomance_monitor.ms) + " ms\n";
    }

    // Build BVH from triangles
    inline void buildBVH() {
        if (triangles.empty()) {
            std::cout << "Cant build BVH, there's no triangles.\n";
            exit(EXIT_FAILURE);
        }

        computeCentersAndBoundingBoxes();

        BVH_Node root = buildBVHSAH2(0, (int)triangles.size() - 1, 1);
        bvh_nodes.push_back(root);
    }

    // Compute bounding boxes and centers for each triangle
    inline void computeCentersAndBoundingBoxes() {
        for (Triangle& triangle : triangles) {
            glm::vec3 min = glm::min(triangle.v0, glm::min(triangle.v1, triangle.v2));
            glm::vec3 max = glm::max(triangle.v0, glm::max(triangle.v1, triangle.v2));

            triangle.bounding_box = AABB(min, max);
            triangle.center = (triangle.v0 + triangle.v1 + triangle.v2) / 3.0f;
        }
    }
    
    // Modified WIP BVH SAH builder with optimizations
    BVH_Node buildBVHSAH2(int start, int end, int primitives_in_node) {
        if (start == end) {
            return BVH_Node(triangles[start].bounding_box, -start - 1, -start - 1);
        }

        if (primitives_in_node > 1) {
            int num_primitives = end - start + 1;
            if (num_primitives <= primitives_in_node) {
                AABB nodeAABB = triangles[start].bounding_box;
                for (int i = start + 1; i <= end; i++) {
                    nodeAABB = mergeAABB(nodeAABB, triangles[i].bounding_box);
                }
                return BVH_Node(nodeAABB, -start - 1, -end - 1);
            }
        }

        // Compute bounds for the entire node
        AABB nodeAABB = triangles[start].bounding_box;
        for (int i = start + 1; i <= end; i++) {
            nodeAABB = mergeAABB(nodeAABB, triangles[i].bounding_box);
        }

        // Find the best split
        int bestSplit = start;
        int bestAxis = 0;
        float bestCost = std::numeric_limits<float>::max();

        for (int axis = 0; axis < 3; axis++) {
            // Sort along this axis
            std::sort(triangles.begin() + start, triangles.begin() + end + 1,
                [axis](const Triangle& a, const Triangle& b) {
                    return a.center[axis] < b.center[axis];
                });

            AABB leftAABB = triangles[start].bounding_box;
            AABB rightAABB = nodeAABB; // Start with the full node AABB

            for (int i = start; i < end; i++) {
                leftAABB = mergeAABB(leftAABB, triangles[i].bounding_box);

                // Adjust the right AABB
                rightAABB.minimum = glm::min(rightAABB.minimum, triangles[i + 1].bounding_box.minimum);
                rightAABB.maximum = glm::max(rightAABB.maximum, triangles[i + 1].bounding_box.maximum);

                float cost = computeCost(leftAABB, i - start + 1) + computeCost(rightAABB, end - i);
                if (cost < bestCost) {
                    bestCost = cost;
                    bestSplit = i;
                    bestAxis = axis;
                }
            }
        }

        // Sort primitives on best axis
        std::sort(triangles.begin() + start, triangles.begin() + end + 1,
            [bestAxis](const Triangle& a, const Triangle& b) {
                return a.center[bestAxis] < b.center[bestAxis];
            });

        BVH_Node leftChild = buildBVHSAH2(start, bestSplit, primitives_in_node);
        bvh_nodes.push_back(leftChild);
        int left_id = (int)bvh_nodes.size() - 1;

        BVH_Node rightChild = buildBVHSAH2(bestSplit + 1, end, primitives_in_node);
        bvh_nodes.push_back(rightChild);
        int right_id = (int)bvh_nodes.size() - 1;

        return BVH_Node(nodeAABB, left_id, right_id);
    }

    
    // deprecated WIP BVH SAH builder (extremely slow, suitable for scenes with <50k triangles)
    BVH_Node buildBVHSAH(int start, int end) {
        if (start == end) {
            return BVH_Node(triangles[start].bounding_box, -start - 1, -start - 1);
        }

        if (end - start == 1) {
            return BVH_Node(mergeAABB(triangles[start].bounding_box, triangles[end].bounding_box), -start - 1, -end - 1);
        }

        // Compute bounds for the entire node
        AABB nodeAABB = triangles[start].bounding_box;
        for (int i = start + 1; i <= end; i++) {
            nodeAABB = mergeAABB(nodeAABB, triangles[i].bounding_box);
        }

        // Find the best split
        int bestSplit = start;
        int bestAxis = 0;
        float bestCost = std::numeric_limits<float>::max();

        for (int axis = 0; axis < 3; axis++) {
            // Sort along this axis
            std::sort(triangles.begin() + start, triangles.begin() + end + 1,
                [axis](const Triangle& a, const Triangle& b) {
                    return a.center[axis] < b.center[axis];
                });

            AABB leftAABB = triangles[start].bounding_box;
            for (int i = start; i < end; i++) {
                leftAABB = mergeAABB(leftAABB, triangles[i].bounding_box);
                AABB rightAABB = triangles[i + 1].bounding_box;
                for (int j = i + 2; j <= end; j++) {
                    rightAABB = mergeAABB(rightAABB, triangles[j].bounding_box);
                }

                float cost = computeCost(leftAABB, i - start + 1) +
                    computeCost(rightAABB, end - i);
                if (cost < bestCost) {
                    bestCost = cost;
                    bestSplit = i;
                    bestAxis = axis;
                }
            }
        }

        // Sort primitives on best axis
        std::sort(triangles.begin() + start, triangles.begin() + end + 1,
            [bestAxis](const Triangle& a, const Triangle& b) {
                return a.center[bestAxis] < b.center[bestAxis];
            });

        BVH_Node leftChild = buildBVHSAH(start, bestSplit);
        bvh_nodes.push_back(leftChild);
        int left_id = (int)bvh_nodes.size() - 1;

        BVH_Node rightChild = buildBVHSAH(bestSplit + 1, end);
        bvh_nodes.push_back(rightChild);
        int right_id = (int)bvh_nodes.size() - 1;

        return BVH_Node(nodeAABB, left_id, right_id);
    }

    // Fast BVH builder (I think it's already deprecated and it's worth using buildBVHSAH2)
    BVH_Node buildBVHFast(int start, int end) {
        if (start == end) {
            return BVH_Node(triangles[start].bounding_box, -start - 1, -start - 1);
        }
        
        if (end - start == 1) {
            return BVH_Node(mergeAABB(triangles[start].bounding_box, triangles[end].bounding_box), -start - 1, -end - 1);
        }

        int axis = rand() % 3;
        std::sort(triangles.begin() + start, triangles.begin() + end, [axis](const Triangle& a, const Triangle& b) {
            return a.bounding_box.minimum[axis] < b.bounding_box.minimum[axis];
            });

        int mid = (start + end) / 2;

        BVH_Node leftChild = buildBVHFast(start, mid);
        bvh_nodes.push_back(leftChild);
        int left_id = (int)bvh_nodes.size() - 1;

        BVH_Node rightChild = buildBVHFast(mid + 1, end);
        bvh_nodes.push_back(rightChild);
        int right_id = (int)bvh_nodes.size() - 1;

        AABB currentAABB = mergeAABB(leftChild.aabb, rightChild.aabb);

        return BVH_Node(currentAABB, left_id, right_id);
    }

private:

    GLuint ssbo_triangles;
    GLuint ssbo_bvh_nodes;

    std::vector<Triangle> triangles;
    std::vector<UniformTriangle> uniform_triangles;
    std::vector<BVH_Node> bvh_nodes;

    bool already_loaded = false;
};