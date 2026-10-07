#pragma once
#include "Buffer.hpp"
#include <Std/Array.hpp>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class IndexBuffer {
          public:
            IndexBuffer() noexcept = default;

            template <typename T>
            inline IndexBuffer(const Array<T>& indices);

            IndexBuffer(const void* data, std::size_t size, std::uint32_t count);

            IndexBuffer(const IndexBuffer&& rhs) = delete;

            IndexBuffer& operator=(const IndexBuffer&& rhs) = delete;

            IndexBuffer(IndexBuffer&& rhs) noexcept;

            IndexBuffer& operator=(IndexBuffer&& rhs) noexcept;

            ~IndexBuffer();

            template <typename T>
            inline void Create(const Array<T>& indices);

            void Create(const void* data, std::size_t size, std::uint32_t count);

            void Bind(VkCommandBuffer commandBuffer) noexcept;

            void Destroy() noexcept;

            std::uint32_t GetCount() const noexcept;

            operator bool() const noexcept;

          private:
            void MoveConstruct(IndexBuffer&& rhs) noexcept;

            void Clear() noexcept;
            ;

          private:
            Buffer m_Buffer;
            std::uint32_t m_IndicesCount;
        };
    } // namespace vk
} // namespace adh

namespace adh {
    namespace vk {
        template <typename T>
        IndexBuffer::IndexBuffer(const Array<T>& indices) {
            Create(indices.GetData(), sizeof(typename Array<T>::Type) * indices.GetSize(), static_cast<std::uint32_t>(indices.GetSize()));
        }

        template <typename T>
        void IndexBuffer::Create(const Array<T>& indices) {
            Create(indices.GetData(), sizeof(typename Array<T>::Type) * indices.GetSize(), static_cast<std::uint32_t>(indices.GetSize()));
        }
    } // namespace vk
} // namespace adh
