#include "VertexBuffer.hpp"
#include "Context.hpp"
#include "Memory.hpp"
#include "Tools.hpp"

#include <cstring>

namespace adh {
    namespace vk {
        VertexBuffer::VertexBuffer(const void* data, std::size_t size) {
            Create(data, size);
        }

        VertexBuffer::VertexBuffer(VertexBuffer&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        VertexBuffer& VertexBuffer::operator=(VertexBuffer&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        VertexBuffer::~VertexBuffer() {
            Clear();
        }

        void VertexBuffer::Create(const void* data, std::size_t size) {
            if (tools::IsUniformMemoryAccess(Context::Get()->GetPhysicalDevice())) {
                m_Buffer.Create(size, 1u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
                void* ptr;
                m_Buffer.Map(nullptr, ptr);
                std::memcpy(ptr, data, size);
            } else {
                Buffer vertexStaging(size, 1u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
                void* ptr;
                vertexStaging.Map(nullptr, ptr);
                std::memcpy(ptr, data, size);
                m_Buffer.Create(size, 1u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                CopyBuffer(vertexStaging, m_Buffer, size);
                vertexStaging.Destroy();
            }
        }

        void VertexBuffer::Bind(VkCommandBuffer commandBuffer) noexcept {
            VkDeviceSize offsets[]{ 0u };
            VkBuffer buffer{ m_Buffer };
            vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &buffer, offsets);
        }

        void VertexBuffer::Destroy() noexcept {
            Clear();
        }

        VertexBuffer::operator bool() const noexcept {
            return m_Buffer != VK_NULL_HANDLE;
        }

        void VertexBuffer::MoveConstruct(VertexBuffer&& rhs) noexcept {
            m_Buffer = Move(rhs.m_Buffer);
        }

        void VertexBuffer::Clear() noexcept {
            m_Buffer.Destroy();
        }
    } // namespace vk
} // namespace adh
