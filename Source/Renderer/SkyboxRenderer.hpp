#pragma once
#include <Math/Math.hpp>
#include <Scene/Components/Skybox.hpp>
#include <Vulkan/DescriptorSet.hpp>
#include <Vulkan/GraphicsPipeline.hpp>
#include <Vulkan/PipelineLayout.hpp>
#include <Vulkan/RenderPass.hpp>
#include <Vulkan/TextureCube.hpp>

#include <string>

namespace adh {
    class SkyboxRenderer {
      public:
        void Create(const vk::RenderPass& scenePass);

        void Prepare(const Skybox* skybox);

        void Draw(VkCommandBuffer cmd, const xmm::Matrix& viewRotationProjection);

      private:
        vk::PipelineLayout m_Layout;
        vk::GraphicsPipeline m_Pipeline;
        vk::DescriptorSet m_Faces;
        vk::TextureCube m_Cube;
        std::string m_Folder;
        float m_Intensity{};
    };
} // namespace adh
