#pragma once
#include "Allocator.hpp"
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Buffer {
          public:
            Buffer() noexcept = default;

            Buffer(std::uint64_t size, std::size_t count, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryProperty);

            Buffer(const Buffer&) = delete;

            Buffer& operator=(const Buffer&) = delete;

            Buffer(Buffer&& rhs) noexcept;

            Buffer& operator=(Buffer&& rhs) noexcept;

            ~Buffer();

            void Create(std::uint64_t size, std::size_t count, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryProperty);

            void Bind() ADH_NOEXCEPT;

            void Map(const void* data) ADH_NOEXCEPT;

            void Map(const void* data, void*& toMap) ADH_NOEXCEPT;

            void Flush() ADH_NOEXCEPT;

            void Destroy() noexcept;

            VkBuffer Get() noexcept;

            VkBuffer Get() const noexcept;

            operator VkBuffer() noexcept;

            operator VkBuffer() const noexcept;

          private:
            void MoveConstruct(Buffer&& rhs) noexcept;

            void Clear() noexcept;

          private:
            BufferData m_Buffer;
        };
    } // namespace vk
} // namespace adh
