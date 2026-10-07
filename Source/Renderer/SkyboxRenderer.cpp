#include "SkyboxRenderer.hpp"

#include <Vulkan/Context.hpp>
#include <Vulkan/Shader.hpp>
#include <Vulkan/Texture2D.hpp>
#include <Vulkan/VertexLayout.hpp>

#include <array>
#include <filesystem>

namespace adh {
    void SkyboxRenderer::Create(const vk::RenderPass& scenePass) {
        m_Layout.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_Layout.CreateSet();
        m_Layout.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(xmm::Matrix), 0);
        m_Layout.AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float), sizeof(xmm::Matrix));
        m_Layout.Create();

        vk::Shader shader("skybox.vert", "skybox.frag");
        vk::VertexLayout vertexLayout;
        vertexLayout.Create();

        const VkPipelineColorBlendAttachmentState blendStates[]{
            vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE),
            vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE),
        };
        m_Pipeline.Create(shader, vertexLayout, m_Layout, scenePass,
                          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                          VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates, { .depthWrite = VK_FALSE });

        m_Faces.Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout, 1u);
        m_Faces.AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        m_Faces.Create(m_Layout.GetSetLayout());
    }

    void SkyboxRenderer::Prepare(const Skybox* skybox) {
        const std::string folder{ skybox ? skybox->folder : std::string{} };
        if (folder != m_Folder) {
            vkDeviceWaitIdle(vk::Context::Get()->GetDevice());
            m_Cube.Destroy();
            m_Folder = folder;
            if (!m_Folder.empty()) {
                const std::string directory{ vk::Context::Get()->GetDataDirectory() + "Assets/Textures/" + m_Folder + "/" };
                std::array<std::string, vk::TextureCube::faceCount> files;
                for (std::size_t face{}; face != files.size(); ++face) {
                    const std::string file{ directory + Skybox::faceNames[face] };
                    files[face] = std::filesystem::exists(file + ".png") ? file + ".png" : file + ".jpg";
                }
                m_Cube.Create(files, *vk::Texture2D::GetDefaultSampler(VK_FILTER_LINEAR));
                m_Faces.Update(m_Cube.GetDescriptor(), 0u, 0u, 0u, 1u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
        }
        m_Intensity = skybox ? skybox->intensity : 0.0f;
    }

    void SkyboxRenderer::Draw(VkCommandBuffer cmd, const xmm::Matrix& viewRotationProjection) {
        if (!m_Folder.empty()) {
            m_Pipeline.Bind(cmd);
            m_Faces.Bind(cmd, 0u);
            vkCmdPushConstants(cmd, m_Layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(viewRotationProjection), &viewRotationProjection);
            vkCmdPushConstants(cmd, m_Layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(viewRotationProjection), sizeof(m_Intensity), &m_Intensity);
            vkCmdDraw(cmd, 36u, 1u, 0u, 0u);
        }
    }
} // namespace adh
