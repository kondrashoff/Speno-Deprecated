#pragma once

#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Utils.h"

class Mesh {
public:

    Mesh() = default;

    Mesh(std::string path, std::string filename) {
        loadFromFile(path, filename);
    }

    void loadFromFile(std::string path, std::string filename) {
        perfomance_monitor.start();

        std::string inputfile = path + filename;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = path; // Path to material files

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(inputfile, reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader error: " << reader.Error();
            }
            exit(EXIT_FAILURE);
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader error: " << reader.Warning();
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

                tinyobj::index_t n0 = mesh.indices[face_index * 3 + 0];
                tinyobj::index_t n1 = mesh.indices[face_index * 3 + 1];
                tinyobj::index_t n2 = mesh.indices[face_index * 3 + 2];

                tinyobj::index_t c0 = mesh.indices[face_index * 3 + 0];

                Triangle triangle;
                Vector3 normal;
                Vector3 vertices[3];

                vertices[0].x = attrib.vertices[3 * idx0.vertex_index + 0];
                vertices[0].y = attrib.vertices[3 * idx0.vertex_index + 1];
                vertices[0].z = attrib.vertices[3 * idx0.vertex_index + 2];
                vertices[1].x = attrib.vertices[3 * idx1.vertex_index + 0];
                vertices[1].y = attrib.vertices[3 * idx1.vertex_index + 1];
                vertices[1].z = attrib.vertices[3 * idx1.vertex_index + 2];
                vertices[2].x = attrib.vertices[3 * idx2.vertex_index + 0];
                vertices[2].y = attrib.vertices[3 * idx2.vertex_index + 1];
                vertices[2].z = attrib.vertices[3 * idx2.vertex_index + 2];

                normal.x = attrib.normals[3 * n0.normal_index + 0];
                normal.y = attrib.normals[3 * n0.normal_index + 1];
                normal.z = attrib.normals[3 * n0.normal_index + 2];

                triangle.color.x = attrib.colors[3 * c0.vertex_index + 0];
                triangle.color.y = attrib.colors[3 * c0.vertex_index + 1];
                triangle.color.z = attrib.colors[3 * c0.vertex_index + 2];

                triangle.v0 = vertices[0];
                triangle.v1 = vertices[1];
                triangle.v2 = vertices[2];

                triangle.normal = normal;

                triangles.push_back(triangle);
            }
        }

        perfomance_monitor.stop();
        std::cerr << "The mesh is successfully loaded from the file. " << perfomance_monitor << std::endl;
    }

    void buildBVH() {
        perfomance_monitor.start();

        createTrianglesBoundingBoxes();

        int axis = 1;
        std::vector<const Triangle*> tempTriangles;
        for (const Triangle& tri : triangles) {
            tempTriangles.push_back(&tri);
        }
        std::sort(tempTriangles.begin(), tempTriangles.end(), [axis](const Triangle* a, const Triangle* b) {
            Vector3 centerA = (a->v0 + a->v1 + a->v2) / 3.0;
            Vector3 centerB = (b->v0 + b->v1 + b->v2) / 3.0;
            return centerA[axis] < centerB[axis];
            });

        if (!triangles.empty()) {
            BVH_Node root = buildBVH(triangles, 0, triangles.size() - 1);
            bvh_nodes.push_back(root);
        }

        perfomance_monitor.stop();
        std::cerr << "BVH tree was builded. " << perfomance_monitor << std::endl;
    }

private:

    void createTrianglesBoundingBoxes() {
        perfomance_monitor.start();

        for (Triangle& triangle : triangles) {
            Vector3 min = Vector3(
                std::min(std::min(triangle.v0.x, triangle.v1.x), triangle.v2.x),
                std::min(std::min(triangle.v0.y, triangle.v1.y), triangle.v2.y),
                std::min(std::min(triangle.v0.z, triangle.v1.z), triangle.v2.z));
            Vector3 max = Vector3(
                std::max(std::max(triangle.v0.x, triangle.v1.x), triangle.v2.x),
                std::max(std::max(triangle.v0.y, triangle.v1.y), triangle.v2.y),
                std::max(std::max(triangle.v0.z, triangle.v1.z), triangle.v2.z));

            triangle.bounding_box = AABB(min, max);
        }

        perfomance_monitor.stop();
        std::cerr << "Triangles bounding boxes were created. " << perfomance_monitor << std::endl;
    }

    BVH_Node buildBVH(const std::vector<Triangle>& triangles, int start, int end) {
        if (start == end) {
            return BVH_Node(triangles[start].bounding_box, -start - 1, -start - 1);
        }

        int mid = (start + end) / 2;

        BVH_Node leftChild = buildBVH(triangles, start, mid);
        bvh_nodes.push_back(leftChild);
        int left_id = bvh_nodes.size() - 1;

        BVH_Node rightChild = buildBVH(triangles, mid + 1, end);
        bvh_nodes.push_back(rightChild);
        int right_id = bvh_nodes.size() - 1;

        AABB currentAABB = mergeAABB(leftChild.aabb, rightChild.aabb);

        return BVH_Node(currentAABB, left_id, right_id);
    }

public:

    std::vector<Triangle> triangles;
    std::vector<BVH_Node> bvh_nodes;

private:

    Perfomance perfomance_monitor;
};