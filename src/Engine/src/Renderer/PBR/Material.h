//
// Created by XuriAjiva on 22.08.2023.
//

#pragma once

#include "defines.h"
#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"

namespace Ajiva::Renderer::PBR {

    class Material {
    private:
        std::vector<Ref<Texture>> textures;
        u64 version = INVALID_ID_U64;
    };

} // Ajiva
// Renderer
