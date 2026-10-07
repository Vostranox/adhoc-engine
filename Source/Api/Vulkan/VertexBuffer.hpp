#pragma once
#include "Buffer.hpp"
#include <Std/Array.hpp>

namespace adh {
    namespace vk {
        class VertexBuffer {
          public:
            VertexBuffer() noexcept = default;

            template <typename T>
            inline VertexBuffer(const Array<T>& vertices);

            VertexBuffer(const void* data, std::size_t size);

            VertexBuffer(const VertexBuffer&& rhs) = delete;

            VertexBuffer& operator=(const VertexBuffer&& rhs) = delete;

            VertexBuffer(VertexBuffer&& rhs) noexcept;

            VertexBuffer& operator=(VertexBuffer&& rhs) noexcept;

            ~VertexBuffer();

            template <typename T>
            inline void Create(const Array<T>& vertices);

            void Create(const void* data, std::size_t size);

            void Bind(VkCommandBuffer commandBuffer) noexcept;

            void Destroy() noexcept;

            operator bool() const noexcept;

          private:
            void MoveConstruct(VertexBuffer&& rhs) noexcept;

            void Clear() noexcept;
            ;

          private:
            Buffer m_Buffer;
        };
    } // namespace vk
} // namespace adh

namespace adh {
    namespace vk {
        template <typename T>
        VertexBuffer::VertexBuffer(const Array<T>& vertices) {
            Create(vertices.GetData(), sizeof(typename Array<T>::Type) * vertices.GetSize());
        }

        template <typename T>
        void VertexBuffer::Create(const Array<T>& vertices) {
            Create(vertices.GetData(), sizeof(typename Array<T>::Type) * vertices.GetSize());
        }
    } // namespace vk
} // namespace adh
