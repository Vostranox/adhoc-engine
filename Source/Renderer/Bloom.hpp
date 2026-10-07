#pragma once
#include "RenderSettings.hpp"

#include <Std/Array.hpp>
#include <Vulkan/DescriptorSet.hpp>
#include <Vulkan/Framebuffer.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/Image.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/Sampler.hpp>

namespace adh {
    constexpr VkFormat hdrFormat{ VK_FORMAT_R16G16B16A16_SFLOAT };

    struct BloomTargets {
        struct Pass {
            VkExtent2D extent{};
            vk::Image image;
            vk::Framebuffer framebuffer;
            vk::DescriptorSet inputs;
            VkDescriptorImageInfo output{};
        };

        Array<Pass> downsample;
        Array<Pass> upsample;
    };

    class Bloom {
      public:
        void Create(const vk::Sampler& sampler, std::uint32_t imageCount);

        void CreateTargets(BloomTargets& targets, VkExtent2D viewExtent, VkDescriptorImageInfo hdrInput);

        void Draw(VkCommandBuffer cmd, std::uint32_t imageIndex, BloomTargets& targets, const RenderSettings& settings);

      private:
        void CreatePass(BloomTargets::Pass& pass, VkExtent2D extent, const vk::PipelineLayout& layout);

        void BeginPass(VkCommandBuffer cmd, std::uint32_t imageIndex, BloomTargets::Pass& pass, vk::GraphicsPipeline& pipeline);

      private:
        const vk::Sampler* m_Sampler{};
        std::uint32_t m_ImageCount{};
        vk::RenderPass m_RenderPass;

        vk::PipelineLayout m_DownsampleLayout;
        vk::GraphicsPipeline m_DownsamplePipeline;

        vk::PipelineLayout m_UpsampleLayout;
        vk::GraphicsPipeline m_UpsamplePipeline;
    };
} // namespace adh
