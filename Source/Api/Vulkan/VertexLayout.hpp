#pragma once
#include <Std/Array.hpp>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class VertexLayout {
          public:
            VertexLayout() noexcept;

            VertexLayout(const VertexLayout& rhs) = delete;

            VertexLayout& operator=(const VertexLayout& rhs) = delete;

            VertexLayout(VertexLayout&& rhs) noexcept;

            VertexLayout& operator=(VertexLayout&& rhs) noexcept;

            ~VertexLayout() = default;

            void AddBinding(std::uint32_t binding, std::uint32_t stride, VkVertexInputRate inputRate) noexcept;

            void AddAttribute(std::uint32_t location, std::uint32_t binding, VkFormat format, std::uint32_t offset) noexcept;

            void Create() noexcept;

            VkPipelineVertexInputStateCreateInfo& Get() noexcept;

            const VkPipelineVertexInputStateCreateInfo& Get() const noexcept;

          private:
            void MoveConstruct(VertexLayout&& rhs) noexcept;

          private:
            Array<VkVertexInputBindingDescription> m_BindingDescription;
            Array<VkVertexInputAttributeDescription> m_AtttributeDescription;
            VkPipelineVertexInputStateCreateInfo m_PipelineVertexInputStateCreateInfo;
        };
    } // namespace vk
} // namespace adh
