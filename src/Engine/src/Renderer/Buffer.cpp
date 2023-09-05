//
// Created by XuriAjiva on 02.08.2023.
//

#include "Buffer.h"
#include "Resource/ResourceManager.h"

#include <utility>


namespace Ajiva::Renderer {
    Buffer::~Buffer() {
        if (cleanUp) {
            buffer.destroy();
            AJ_RegisterDestroyed(this, typeid(Buffer));
        }
    }

    Buffer::Buffer(wgpu::Buffer buffer, u64 size, u64 alignedSize, Ref<wgpu::Queue> queue, bool cleanUp) :
            buffer(buffer), m_queue(std::move(queue)), cleanUp(cleanUp), size(size), alignedSize(alignedSize) {
        AJ_RegisterCreated(this, typeid(Buffer));
    }

    void Buffer::UpdateBufferData(const void *data, uint64_t updateSize, uint64_t offset) {
        if (updateSize == INVALID_ID_U64)
            updateSize = this->size;
        if (offset + updateSize > this->size) {
            PLOG_WARNING << "Buffer::UpdateBufferData: updateSize + offset > this->size: " << updateSize << " + "
                         << offset << " > " << this->size;
            updateSize = this->size - offset;
        }
        m_queue->writeBuffer(buffer, offset, data, ALIGN_AT(updateSize, 4));
    }

} // Ajiva::Renderer