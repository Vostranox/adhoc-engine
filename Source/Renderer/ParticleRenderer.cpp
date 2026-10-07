#include "ParticleRenderer.hpp"

#include <Event/Event.hpp>
#include <Scene/Components/ParticleEmitter.hpp>
#include <Utility.hpp>
#include <Vulkan/Shader.hpp>
#include <Vulkan/VertexLayout.hpp>

#include <algorithm>
#include <cstddef>
#include <string>

namespace adh {
    void ParticleRenderer::Create(const vk::RenderPass& scenePass, std::uint32_t imageCount) {
        m_Layout.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Camera), 0);
        m_Layout.Create();

        vk::Shader shader("particle.vert", "particle.frag");
        vk::VertexLayout vertexLayout;
        vertexLayout.AddBinding(0, sizeof(Instance), VK_VERTEX_INPUT_RATE_INSTANCE);
        vertexLayout.AddAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, ADH_OFFSET(Instance, position));
        vertexLayout.AddAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, ADH_OFFSET(Instance, color));
        vertexLayout.Create();

        VkPipelineColorBlendAttachmentState additive{};
        additive.blendEnable         = VK_TRUE;
        additive.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        additive.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        additive.colorBlendOp        = VK_BLEND_OP_ADD;
        additive.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        additive.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        additive.alphaBlendOp        = VK_BLEND_OP_ADD;
        additive.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        auto noEntityIds{ vk::initializers::PipelineColorBlendAttachmentState(VK_FALSE) };
        noEntityIds.colorWriteMask = 0u;
        const VkPipelineColorBlendAttachmentState blendStates[]{ additive, noEntityIds };
        m_Pipeline.Create(shader, vertexLayout, m_Layout, scenePass,
                          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                          VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, blendStates, { .depthWrite = VK_FALSE });

        m_Instances.Create(nullptr, maxParticles * sizeof(Instance), imageCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    void ParticleRenderer::Upload(ecs::World& world, std::uint32_t imageIndex) {
        auto* instances{ reinterpret_cast<Instance*>(static_cast<std::byte*>(m_Instances.GetMappedPtr()) + m_Instances.GetDescriptor(imageIndex).offset) };
        m_Count = 0u;
        world.get_system<ParticleEmitter>().for_each([&](ParticleEmitter& emitter) {
            for (const auto& particle : emitter.particles) {
                if (m_Count == maxParticles) {
                    if (!m_HasReportedLimit) {
                        m_HasReportedLimit = true;
                        const std::string message{ "The scene has more than " + std::to_string(maxParticles) + " particles, only that many are drawn\n" };
                        EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, message.c_str());
                    }
                    return;
                }
                const float alpha{ emitter.lifetime > 0.0f ? std::clamp(1.0f - particle.age / emitter.lifetime, 0.0f, 1.0f) : 0.0f };
                instances[m_Count++] = { particle.position, emitter.size, emitter.color * emitter.intensity, alpha };
            }
        });
    }

    void ParticleRenderer::Draw(VkCommandBuffer cmd, std::uint32_t imageIndex, const xmm::Matrix& viewProjection, const Vector3D& right, const Vector3D& up) {
        if (m_Count != 0u) {
            const Camera camera{ viewProjection, Vector4D{ right.x, right.y, right.z, 0.0f }, Vector4D{ up.x, up.y, up.z, 0.0f } };
            const VkBuffer buffer{ m_Instances };
            const VkDeviceSize offset{ m_Instances.GetDescriptor(imageIndex).offset };

            m_Pipeline.Bind(cmd);
            vkCmdBindVertexBuffers(cmd, 0u, 1u, &buffer, &offset);
            vkCmdPushConstants(cmd, m_Layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(camera), &camera);
            vkCmdDraw(cmd, 6u, m_Count, 0u, 0u);
        }
    }
} // namespace adh
