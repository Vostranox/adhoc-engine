#pragma once
#include <Event/Event.hpp>
#include <Math/Math.hpp>
#include <Std/Array.hpp>
#include <Vulkan/DescriptorSet.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/Image.hpp>
#include <Vulkan/Initializers.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/Sampler.hpp>
#include <Vulkan/Scissor.hpp>
#include <Vulkan/Texture2D.hpp>
#include <Vulkan/Tools.hpp>
#include <Vulkan/Viewport.hpp>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        struct ImGuiBuffers {
            Buffer buffer;
            VkDeviceSize count;
            void* mappedPtr;
        };

        class VulkanImGui {
          private:
            using DescriptorIndexMap = std::unordered_map<std::string, std::uint32_t>;

          public:
            static constexpr std::uint32_t maxTextures{ 100u };

          public:
            void Create(const RenderPass& renderPass, VkQueue queue, std::uint32_t imageViewCount);

            [[nodiscard]] void* AddTexture(const std::string& name, VkImageView imageView, const vk::Sampler& sampler);

            void UpdateTexture(const std::string& name, VkImageView imageView, const vk::Sampler& sampler);

            void Draw(VkCommandBuffer commandBuffer, std::uint32_t imageIndex);

          private:
            PipelineLayout m_PipelineLayout;
            Array<DescriptorSet> m_DescriptorSet;
            DescriptorIndexMap m_DescriptorIndices;
            GraphicsPipeline m_GraphicsPipeline;
            Texture2D m_FontTexture;
            Sampler m_FontSampler;
            Array<ImGuiBuffers> m_VertexBuffer;
            Array<ImGuiBuffers> m_IndexBuffer;
            vk::Viewport m_Viewport;
            vk::Scissor m_Scissor;
            std::uint32_t m_ImageViewCount;

            struct {
                Vector2D scale;
                Vector2D translate;
            } m_PushConstantData{};
        };
    } // namespace vk
} // namespace adh
