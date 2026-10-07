#include "Buffer.hpp"
#include <Std/Utility.hpp>

namespace adh {
    namespace vk {
        Buffer::Buffer(std::uint64_t size, std::size_t count, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryProperty) {
            Create(size, count, usage, memoryProperty);
        }

        Buffer::Buffer(Buffer&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Buffer& Buffer::operator=(Buffer&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        Buffer::~Buffer() {
            Clear();
        }

        void Buffer::Create(std::uint64_t size, std::size_t count, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryProperty) {
            ADH_THROW(m_Buffer.buffer == VK_NULL_HANDLE, "Calling create on an already created buffer!");
            Allocator::CreateBuffer(static_cast<VkDeviceSize>(size * count),
                                    usage,
                                    memoryProperty,
                                    m_Buffer);
            Bind();
        }

        void Buffer::Bind() ADH_NOEXCEPT {
            Allocator::BindBuffer(m_Buffer);
        }

        void Buffer::Map(const void* data) ADH_NOEXCEPT {
            void* ptr;
            Allocator::Map(data, m_Buffer.memoryData.tail - m_Buffer.memoryData.head, m_Buffer.memoryData, ptr);
        }

        void Buffer::Map(const void* data, void*& toMap) ADH_NOEXCEPT {
            Allocator::Map(data, m_Buffer.memoryData.tail - m_Buffer.memoryData.head, m_Buffer.memoryData, toMap);
        }

        void Buffer::Flush() ADH_NOEXCEPT {
            Allocator::Flush(m_Buffer.memoryData.tail - m_Buffer.memoryData.head, m_Buffer.memoryData.head, m_Buffer.memoryData.memoryBlock->deviceMemory);
        }

        void Buffer::Destroy() noexcept {
            Clear();
        }

        VkBuffer Buffer::Get() noexcept {
            return m_Buffer.buffer;
        }

        VkBuffer Buffer::Get() const noexcept {
            return m_Buffer.buffer;
        }

        Buffer::operator VkBuffer() noexcept {
            return m_Buffer.buffer;
        }

        Buffer::operator VkBuffer() const noexcept {
            return m_Buffer.buffer;
        }

        void Buffer::MoveConstruct(Buffer&& rhs) noexcept {
            m_Buffer     = rhs.m_Buffer;
            rhs.m_Buffer = {};
        }

        void Buffer::Clear() noexcept {
            if (m_Buffer.buffer != VK_NULL_HANDLE) {
                Allocator::DestroyBuffer(m_Buffer);
                m_Buffer = {};
            }
        }
    } // namespace vk
} // namespace adh
