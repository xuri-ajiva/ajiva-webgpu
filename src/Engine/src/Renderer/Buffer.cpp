//
// Created by XuriAjiva on 02.08.2023.
//

#include "Buffer.h"

#include <utility>


namespace Ajiva::Renderer {
    Buffer::~Buffer() {
        if (cleanUp)
            buffer.destroy();
    }

    Buffer::Buffer(wgpu::Buffer buffer, Ref<wgpu::Queue> queue, bool cleanUp) :
            buffer(buffer), m_queue(std::move(queue)), cleanUp(cleanUp) {}

    void Buffer::UpdateBufferData(const void *data, uint64_t size) {
        m_queue->writeBuffer(buffer, 0, data, ALIGN_AT(size, 4));
    }

} // Ajiva::Renderer