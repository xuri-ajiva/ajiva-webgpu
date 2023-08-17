//
// Created by XuriAjiva on 03.08.2023.
//
#pragma once

#include "defines.h"
#include <filesystem>
#include "Renderer/GpuContext.h"
#include "stb_image.h"
#include "tiny_obj_loader.h"

namespace Ajiva::Resource {
    class AJ_API Loader {
    public:
        Loader() = default;
        explicit Loader(std::filesystem::path resourceDirectory) : resourceDirectory(std::move(resourceDirectory)) {}

        std::string LoadFile(const std::filesystem::path &path, bool throwOnFail = true);


        bool LoadGeometryFromSimpleTxt(const std::filesystem::path &resourcePath, std::vector<Renderer::VertexData> &pointData,
                                       std::vector<uint16_t> &indexData);

        bool LoadGeometryFromObj(const std::filesystem::path &resourcePath, std::vector<Renderer::VertexData> &pointData,
                                 std::vector<uint16_t> &indexData);


        Ref<Renderer::Texture>
        LoadTexture(const std::filesystem::path &resourcePath, const Renderer::GpuContext &context, uint32_t mipLevelCount = 0);

    private:
        std::filesystem::path resourceDirectory;
    };
} // Ajiva
