//
// Created by XuriAjiva on 03.08.2023.
//
#pragma once

#include "defines.h"
#include <filesystem>
#include "Renderer/GpuContext.h"

namespace Ajiva::Resource {
    class Loader {
    public:

        static std::string LoadFile(const std::filesystem::path &path, bool throwOnFail = true);

        static bool
        LoadGeometryFromSimpleTxt(const std::filesystem::path &path, std::vector<Renderer::VertexData> &pointData,
                                  std::vector<uint16_t> &indexData);

        static bool LoadGeometryFromObj(const std::filesystem::path &path, std::vector<Renderer::VertexData> &pointData,
                                        std::vector<uint16_t> &indexData);

        static Ref<Renderer::Texture>
        LoadTexture(const std::filesystem::path &path, const Renderer::GpuContext &context, uint32_t mipLevelCount = 0);
    };
} // Ajiva
