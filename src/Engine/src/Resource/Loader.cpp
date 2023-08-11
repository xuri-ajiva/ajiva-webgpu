//
// Created by XuriAjiva on 03.08.2023.
//

#include "Loader.h"

#include <fstream>
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION

#include "tiny_obj_loader.h"

namespace Ajiva::Resource {
    bool Loader::LoadGeometryFromSimpleTxt(const std::filesystem::path &path,
                                           std::vector<Renderer::VertexData> &pointData,
                                           std::vector<uint16_t> &indexData) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        pointData.clear();
        indexData.clear();

        enum class Section {
            None,
            Points,
            Indices,
        };
        Section currentSection = Section::None;

        float value;
        uint16_t index;
        std::string line;
        while (!file.eof()) {
            getline(file, line);

            // overcome the `CRLF` problem
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line == "[points]") {
                currentSection = Section::Points;
            } else if (line == "[indices]") {
                currentSection = Section::Indices;
            } else if (line[0] == '#' || line.empty()) {
                // Do nothing, this is a comment
            } else if (currentSection == Section::Points) {
                std::istringstream iss(line);
                Renderer::VertexData vertex(iss);

                pointData.push_back(vertex);
            } else if (currentSection == Section::Indices) {
                std::istringstream iss(line);
                // Get corners #0 #1 and #2
                for (int i = 0; i < 3; ++i) {
                    iss >> index;
                    indexData.push_back(index);
                }
            }
        }
        return true;
    }

    bool Loader::LoadGeometryFromObj(const std::filesystem::path &path, std::vector<Renderer::VertexData> &pointData,
                                     std::vector<uint16_t> &indexData) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        // Call the core loading procedure of TinyOBJLoader
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

        // Check errors
        if (!warn.empty()) {
            PLOG_WARNING << warn;
        }

        if (!err.empty()) {
            PLOG_ERROR << err;
        }

        if (!ret) {
            return false;
        }

        // Filling in vertexData:
        pointData.clear();
        for (const auto &shape: shapes) {
            size_t offset = pointData.size();
            pointData.resize(offset + shape.mesh.indices.size());

            for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
                const tinyobj::index_t &idx = shape.mesh.indices[i];

                pointData[offset + i].position = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        -attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
                        attrib.vertices[3 * idx.vertex_index + 1]
                };

                // Also apply the transform to normals!!
                pointData[offset + i].normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        -attrib.normals[3 * idx.normal_index + 2],
                        attrib.normals[3 * idx.normal_index + 1]
                };

                pointData[offset + i].color = {
                        attrib.colors[3 * idx.vertex_index + 0],
                        attrib.colors[3 * idx.vertex_index + 1],
                        attrib.colors[3 * idx.vertex_index + 2]
                };

                pointData[offset + i].uv = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1 - attrib.texcoords[2 * idx.texcoord_index + 1] // Flip Y coord due to different conventions
                };
            }
        }

        return true;
    }

    std::string Loader::LoadFile(const std::filesystem::path &path, bool throwOnFail) {
        std::ifstream file(path);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        if (content.empty() && throwOnFail) {
            PLOG_FATAL << "Failed to load shader from file: " << path;
            AJ_FAIL("Failed to load shader!");
        }
        return content;
    }
} // Ajiva
// Resource