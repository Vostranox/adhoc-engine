#include "IndexBuffer.hpp"
#include "Context.hpp"
#include "Memory.hpp"
#include "Tools.hpp"

#include <cstring>

namespace adh {
    namespace vk {
        IndexBuffer::IndexBuffer(const void* data, std::size_t size, std::uint32_t count) {
            Create(data, size, count);
        }

        IndexBuffer::IndexBuffer(IndexBuffer&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        IndexBuffer& IndexBuffer::operator=(IndexBuffer&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        IndexBuffer::~IndexBuffer() {
            Clear();
        }

        void IndexBuffer::Create(const void* data, std::size_t size, std::uint32_t count) {
            if (tools::IsUniformMemoryAccess(Context::Get()->GetPhysicalDevice())) {
                m_Buffer.Create(size, 1u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
                void* ptr;
                m_Buffer.Map(nullptr, ptr);
                std::memcpy(ptr, data, size);
            } else {
                Buffer indexStaging(size, 1u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
                void* ptr;
                indexStaging.Map(nullptr, ptr);
                std::memcpy(ptr, data, size);
                m_Buffer.Create(size, 1u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                CopyBuffer(indexStaging, m_Buffer, size);
                indexStaging.Destroy();
            }

            m_IndicesCount = count;
        }

        void IndexBuffer::Bind(VkCommandBuffer commandBuffer) noexcept {
            vkCmdBindIndexBuffer(commandBuffer, m_Buffer, 0u, VK_INDEX_TYPE_UINT32);
        }

        void IndexBuffer::Destroy() noexcept {
            Clear();
        }

        std::uint32_t IndexBuffer::GetCount() const noexcept {
            return m_IndicesCount;
        }

        IndexBuffer::operator bool() const noexcept {
            return m_Buffer != VK_NULL_HANDLE;
        }

        void IndexBuffer::MoveConstruct(IndexBuffer&& rhs) noexcept {
            m_Buffer       = Move(rhs.m_Buffer);
            m_IndicesCount = rhs.m_IndicesCount;
        }

        void IndexBuffer::Clear() noexcept {
            m_Buffer.Destroy();
        }
    } // namespace vk
} // namespace adh
