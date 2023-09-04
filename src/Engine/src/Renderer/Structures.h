//
// Created by XuriAjiva on 22.08.2023.
//

#pragma once

#include "defines.h"
#include "glm/glm.hpp"

namespace Ajiva::Renderer {
    using namespace glm;

    struct VertexData { //move to resource
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;

        VertexData(std::istringstream &s) {
            s >> position.x;
            s >> position.y;
            s >> position.z;
            s >> normal.x;
            s >> normal.y;
            s >> normal.z;
            s >> color.r;
            s >> color.g;
            s >> color.b;
            s >> uv.x;
            s >> uv.y;
        }

        VertexData(glm::vec3 position, glm::vec3 normal, glm::vec3 color) : position(position), normal(normal),
                                                                            color(color) {}

        VertexData() = default;
    };

    struct UniformData {
        glm::mat4x4 projectionMatrix;
        glm::mat4x4 viewMatrix;
        glm::vec3 worldPos;
        glm::vec2 resolution;
        float time;
        float gamma;
        float padding[1];
    };
    static_assert(sizeof(UniformData) % 16 == 0);

    struct InstanceData {
        glm::mat4x4 modelMatrix;
        glm::vec4 color;
    };
    static_assert(sizeof(InstanceData) % 16 == 0);

    struct Light {
        glm::vec4 position;
        glm::vec4 color;
    };
    static_assert(sizeof(Light) == 32);

    struct LightningUniform {
        Light lights[4];
        glm::vec4 ambient;
        float hardness;
        float kd;
        float ks;
        float padding;
    };
    static_assert(sizeof(LightningUniform) % 16 == 0);

    struct Material {
        glm::vec3 baseColor;
        float metallic;
        float specular;         // Specular reflection, calculated from metallic
        float roughness;
        float ior;              // Index of Refraction

        glm::vec3 emission;
        float alpha;            // Opacity (1.0 - fully opaque, 0.0 - fully transparent)
        float sheen;            // Sheen intensity
        //glm::vec3 anisotropic;  // Anisotropic reflection factor
    };
    static_assert(sizeof(Material) % 16 == 0);
}