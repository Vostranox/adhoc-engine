#pragma once
#include <Vulkan/DescriptorSet.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/Image.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>

namespace adh {
    void CreateColorPass(vk::RenderPass& renderPass, VkFormat format);

    vk::Image CreateColorTarget(VkExtent2D extent, VkFormat format);

    void CreateFullscreenPipeline(vk::GraphicsPipeline& pipeline, const vk::PipelineLayout& layout, const vk::RenderPass& renderPass, const char* fragmentShader);

    void UpdateInputImage(vk::DescriptorSet& set, std::uint32_t binding, const VkDescriptorImageInfo& image);
} // namespace adh
