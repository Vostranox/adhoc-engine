#pragma once
#include "PipelineLayout.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"
#include <span>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        struct DepthStencilState {
            VkBool32 depthTest{ VK_TRUE };
            VkBool32 depthWrite{ VK_TRUE };
            VkCompareOp depthCompareOp{ VK_COMPARE_OP_LESS_OR_EQUAL };
            VkBool32 stencilTest{ VK_FALSE };
            VkStencilOpState stencil{};
            VkBool32 depthClamp{ VK_FALSE };
        };

        class GraphicsPipeline {
          public:
            GraphicsPipeline() noexcept;

            GraphicsPipeline(
                const Shader& shader,
                const VertexLayout& vertexLayout,
                const PipelineLayout& layout,
                const RenderPass& renderPass,
                VkPrimitiveTopology topology,
                VkCullModeFlagBits cullMode,
                VkFrontFace frontFace,
                VkSampleCountFlagBits rasterizationSamples,
                VkBool32 sampleShadingEnable,
                float minSampleShading,
                VkBool32 enableBlending);

            GraphicsPipeline(VkGraphicsPipelineCreateInfo createInfo);

            GraphicsPipeline(const GraphicsPipeline& rhs) = delete;

            GraphicsPipeline& operator=(const GraphicsPipeline& rhs) = delete;

            GraphicsPipeline(GraphicsPipeline&& rhs) noexcept;

            GraphicsPipeline& operator=(GraphicsPipeline&& rhs) noexcept;

            ~GraphicsPipeline();

            void Create(
                const Shader& shader,
                const VertexLayout& vertexLayout,
                const PipelineLayout& layout,
                const RenderPass& renderPass,
                VkPrimitiveTopology topology,
                VkCullModeFlagBits cullMode,
                VkFrontFace frontFace,
                VkSampleCountFlagBits rasterizationSamples,
                VkBool32 sampleShadingEnable,
                float minSampleShading,
                VkBool32 enableBlending);

            void Create(
                const Shader& shader,
                const VertexLayout& vertexLayout,
                const PipelineLayout& layout,
                const RenderPass& renderPass,
                VkPrimitiveTopology topology,
                VkCullModeFlagBits cullMode,
                VkFrontFace frontFace,
                VkSampleCountFlagBits rasterizationSamples,
                VkBool32 sampleShadingEnable,
                float minSampleShading,
                std::span<const VkPipelineColorBlendAttachmentState> colorBlendAttachments,
                const DepthStencilState& depthStencil = {});

            void Create(VkGraphicsPipelineCreateInfo createInfo);

            void Bind(VkCommandBuffer commandBuffer);

            VkPipeline Get() noexcept;

            VkPipeline Get() const noexcept;

            void Destroy() noexcept;

            operator VkPipeline() noexcept;

            operator VkPipeline() const noexcept;

          private:
            void MoveConstruct(GraphicsPipeline&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkPipeline m_Pipeline;
        };
    } // namespace vk
} // namespace adh
