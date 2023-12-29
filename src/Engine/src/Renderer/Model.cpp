//
// Created by XuriAjiva on 05.09.2023.
//

#include "Model.h"

#include <utility>

Ajiva::Renderer::Model::Model(
    u64 id,
    const std::vector<Ajiva::Renderer::VertexData>& vertexData,
    const std::vector<u16>& indexData,
    const Ajiva::Ref<Buffer>& vertexBuffer,
    const Ajiva::Ref<Buffer>& indexBuffer)
    : id(id),
      vertexData(vertexData),
      indexData(indexData),
      vertexBuffer(vertexBuffer),
      indexBuffer(indexBuffer)
{
}

