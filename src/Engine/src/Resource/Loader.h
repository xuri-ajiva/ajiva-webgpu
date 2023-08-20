//
// Created by XuriAjiva on 03.08.2023.
//
#pragma once

#include "defines.h"
#include <filesystem>
#include <utility>
#include "Renderer/GpuContext.h"
#include "stb_image.h"
#include "tiny_obj_loader.h"
#include "Core/ThreadPool.h"

namespace Ajiva::Resource {
    class AJ_API Loader {
    public:
        Loader() = default;
        explicit Loader(std::filesystem::path resourceDirectory, Ref<Core::IThreadPool> threadPool)
        : resourceDirectory(std::move(resourceDirectory)), threadPool(std::move(threadPool)) {}

        std::string LoadFile(const std::filesystem::path &path, bool throwOnFail = true);


        bool LoadGeometryFromSimpleTxt(const std::filesystem::path &resourcePath, std::vector<Renderer::VertexData> &pointData,
                                       std::vector<uint16_t> &indexData);

        bool LoadGeometryFromObj(const std::filesystem::path &resourcePath, std::vector<Renderer::VertexData> &pointData,
                                 std::vector<uint16_t> &indexData);

        Ref<Renderer::Texture>
        LoadTexture(const std::filesystem::path &resourcePath, const Renderer::GpuContext &context, uint32_t mipLevelCount = 0);

        Ref<Renderer::Texture>
        LoadTextureAsync(const std::filesystem::path &resourcePath, const Renderer::GpuContext &context, uint32_t mipLevelCount = 0);

    private:
        std::filesystem::path resourceDirectory;
        Ref<Core::IThreadPool> threadPool;
    };
} // Ajiva
