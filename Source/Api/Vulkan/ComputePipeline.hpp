#pragma once
#include <Utility.hpp>

#include <cstdint>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class ComputePipeline {
          public:
            ComputePipeline() noexcept;

            ComputePipeline(VkPipelineLayout layout, VkPipelineShaderStageCreateInfo stage) ADH_NOEXCEPT;

            ComputePipeline(const ComputePipeline& rhs) = delete;

            ComputePipeline& operator=(const ComputePipeline& rhs) = delete;

            ComputePipeline(ComputePipeline&& rhs) noexcept;

            ComputePipeline& operator=(ComputePipeline&& rhs) noexcept;

            ~ComputePipeline();

            void Create(VkPipelineLayout layout, VkPipelineShaderStageCreateInfo stage) ADH_NOEXCEPT;

            void Bind(VkCommandBuffer commandBuffer) noexcept;

            void Dispatch(VkCommandBuffer commandBuffer, std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept;

            static std::uint32_t GroupCount(std::uint32_t count, std::uint32_t localSize) noexcept;

            void Destroy() noexcept;

            operator VkPipeline() noexcept;

            operator VkPipeline() const noexcept;

          private:
            void MoveConstruct(ComputePipeline&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkPipeline m_Pipeline;
        };
    } // namespace vk
} // namespace adh
