#include "SunShadows.hpp"
#include "SceneRenderer.hpp"

#include <Scene/Components/Camera.hpp>
#include <Vertex.hpp>
#include <Vulkan/Attachments.hpp>
#include <Vulkan/Context.hpp>
#include <Vulkan/Scissor.hpp>
#include <Vulkan/Shader.hpp>
#include <Vulkan/Subpass.hpp>
#include <Vulkan/Tools.hpp>
#include <Vulkan/VertexLayout.hpp>
#include <Vulkan/Viewport.hpp>

#include <algorithm>
#include <cmath>
#include <iterator>

namespace adh {
    constexpr float cascadeBlend{ 0.1f };
    constexpr float normalOffsetTexels{ 1.5f };
    constexpr float slopeBias{ 1.5f };

    const xmm::Matrix uvFromClip{
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f
    };

    static Vector3D TransformPoint(const xmm::Matrix& m, const Vector3D& p) noexcept {
        float out[4];
        for (int row{}; row != 4; ++row) {
            out[row] = m.f[row] * p.x + m.f[4 + row] * p.y + m.f[8 + row] * p.z + m.f[12 + row];
        }
        return Vector3D{ out[0] / out[3], out[1] / out[3], out[2] / out[3] };
    }

    struct Sphere {
        Vector3D center;
        float radius;
    };

    static Sphere SliceSphere(const Vector3D (&nearCorners)[4], const Vector3D (&farCorners)[4]) noexcept {
        const Vector3D nearCenter{ (nearCorners[0] + nearCorners[1] + nearCorners[2] + nearCorners[3]) * 0.25f };
        const Vector3D farCenter{ (farCorners[0] + farCorners[1] + farCorners[2] + farCorners[3]) * 0.25f };
        const float nearRadius2{ math::distance_squared(nearCenter, nearCorners[0]) };
        const float farRadius2{ math::distance_squared(farCenter, farCorners[0]) };
        const float length2{ math::distance_squared(nearCenter, farCenter) };
        const float t{ length2 > 0.0f ? std::clamp((length2 + farRadius2 - nearRadius2) / (2.0f * length2), 0.0f, 1.0f) : 0.5f };

        Sphere sphere{ math::lerp(nearCenter, farCenter, t), 0.0f };
        for (int i{}; i != 4; ++i) {
            sphere.radius = std::max({ sphere.radius, math::distance_squared(sphere.center, nearCorners[i]), math::distance_squared(sphere.center, farCorners[i]) });
        }
        sphere.radius = std::sqrt(sphere.radius);
        return sphere;
    }

    void SunShadows::Create() {
        const VkPhysicalDevice physicalDevice{ vk::Context::Get()->GetPhysicalDevice() };
        ADH_THROW(vk::tools::GetPhysicalDeviceFeatures(physicalDevice).depthClamp, "The sun's shadow maps need depthClamp!");
        const VkFormat format{ vk::tools::GetSupportedShadowMapFormat(physicalDevice) };
        const VkExtent2D extent{ mapSize, mapSize };

        m_Maps.Create(
            { mapSize, mapSize, 1u },
            format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_TYPE_2D,
            VkImageCreateFlagBits(0),
            1u,
            maxCascades,
            VK_SAMPLE_COUNT_1_BIT,
            VkImageUsageFlagBits(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT),
            VK_IMAGE_ASPECT_DEPTH_BIT,
            VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SHARING_MODE_EXCLUSIVE);

        vk::Attachment attachment;
        attachment.AddDescription(
            format,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            vk::Attachment::Type::eDepth);

        vk::Subpass subpass;
        subpass.AddDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, attachment);
        subpass.AddDependencies(
            VK_SUBPASS_EXTERNAL,
            0u,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VkPipelineStageFlagBits(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
            VkAccessFlagBits(0),
            VkAccessFlagBits(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
        subpass.AddDependencies(
            0u,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT);

        Array<VkClearValue> clearValues;
        clearValues.Resize(1u);
        clearValues[0].depthStencil = { 1.0f, 0u };
        m_Pass.Create(attachment, subpass, { {}, extent }, Move(clearValues));

        for (std::uint32_t layer{}; layer != maxCascades; ++layer) {
            m_LayerViews[layer].Create(m_Maps.GetImage(), VK_IMAGE_VIEW_TYPE_2D, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1u, 1u, layer);
            VkImageView attachments[]{ m_LayerViews[layer].Get() };
            m_Framebuffers[layer].Create(m_Pass, std::size(attachments), attachments, extent, 1u);
        }

        const bool linear{ (vk::tools::GetPhysicalDeviceFormatProperties(physicalDevice, format).optimalTilingFeatures &
                            VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0u };
        m_Sampler.Create(linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                         VK_COMPARE_OP_LESS_OR_EQUAL);

        m_Layout.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(xmm::Matrix), 0u);
        m_Layout.Create();

        vk::Shader shader("shadowmap.vert");
        vk::VertexLayout vertexLayout;
        vertexLayout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
        vertexLayout.AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, ADH_OFFSET(Vertex, position));
        vertexLayout.Create();
        m_Pipeline.Create(shader, vertexLayout, m_Layout, m_Pass, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                          VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, {}, { .depthClamp = VK_TRUE });
    }

    SunCascades SunShadows::Fit(const SceneCamera& camera, const RenderSettings& settings) noexcept {
        SunCascades cascades;
        SunShadowData& data{ cascades.data };
        const int count{ std::clamp(settings.shadowCascades, 0, static_cast<int>(maxCascades)) };
        data.cascadeCount = count;
        data.showCascades = settings.showShadowCascades;
        if (count == 0) {
            return cascades;
        }

        const xmm::Matrix inverseProjection{ xmm::inverse(camera.projection) };
        Vector3D nearCorners[4];
        Vector3D farCorners[4];
        for (int i{}; i != 4; ++i) {
            const float x{ (i & 1) ? 1.0f : -1.0f };
            const float y{ (i & 2) ? 1.0f : -1.0f };
            nearCorners[i] = TransformPoint(inverseProjection, Vector3D{ x, y, 0.0f });
            farCorners[i]  = TransformPoint(inverseProjection, Vector3D{ x, y, 1.0f });
        }
        const float nearZ{ nearCorners[0].z };
        const float farZ{ farCorners[0].z };
        const float shadowEnd{ std::min(std::max(settings.shadowDistance, nearZ + 0.01f), farZ) };
        const bool orthographic{ camera.projection.f[15] == 1.0f };
        const float lambda{ orthographic ? 0.0f : std::clamp(settings.shadowSplitLambda, 0.0f, 1.0f) };
        const float softness{ std::max(settings.shadowSoftness, 0.0f) };
        const xmm::Matrix lightView{ XmmLookAt(Vector3D{}, Vector3D{} - settings.sunPosition, Vector3D{ 0.0f, 1.0f, 0.0f }) };
        const xmm::Matrix viewToLight{ lightView * xmm::inverse(camera.view) };

        float begin{ nearZ };
        for (int c{}; c != count; ++c) {
            const float t{ static_cast<float>(c + 1) / static_cast<float>(count) };
            const float uniformEnd{ nearZ + (shadowEnd - nearZ) * t };
            const float logarithmicEnd{ orthographic ? uniformEnd : nearZ * std::pow(shadowEnd / nearZ, t) };
            const float end{ uniformEnd + (logarithmicEnd - uniformEnd) * lambda };
            data.cascadeEnd[c]        = end;
            data.cascadeBlendStart[c] = end - cascadeBlend * (end - begin);

            const float sliceBegin{ c == 0 ? nearZ : data.cascadeBlendStart[c - 1] };
            Vector3D sliceNear[4];
            Vector3D sliceFar[4];
            for (int i{}; i != 4; ++i) {
                sliceNear[i] = math::lerp(nearCorners[i], farCorners[i], (sliceBegin - nearZ) / (farZ - nearZ));
                sliceFar[i]  = math::lerp(nearCorners[i], farCorners[i], (end - nearZ) / (farZ - nearZ));
            }
            Sphere sphere{ SliceSphere(sliceNear, sliceFar) };

            const float texel{ 2.0f * sphere.radius / static_cast<float>(mapSize) };
            const float normalOffset{ normalOffsetTexels * texel + softness };
            sphere.radius += normalOffset + softness;

            const float mapTexel{ 2.0f * sphere.radius / static_cast<float>(mapSize) };
            Vector3D center{ TransformPoint(viewToLight, sphere.center) };
            center.x = std::floor(center.x / mapTexel) * mapTexel;
            center.y = std::floor(center.y / mapTexel) * mapTexel;
            const xmm::Matrix projection{ xmm::orthographic_lh(center.x - sphere.radius, center.x + sphere.radius, center.y - sphere.radius,
                                                               center.y + sphere.radius, center.z - sphere.radius, center.z + sphere.radius) };

            cascades.viewProjection[c]  = projection * lightView;
            data.cascadeLightSpace[c]   = uvFromClip * cascades.viewProjection[c];
            data.cascadeFilterRadius[c] = softness / (2.0f * sphere.radius);
            data.cascadeNormalOffset[c] = normalOffset;
            begin                       = end;
        }

        data.cameraForward[0] = camera.view.f[2];
        data.cameraForward[1] = camera.view.f[6];
        data.cameraForward[2] = camera.view.f[10];
        return cascades;
    }

    void SunShadows::Render(VkCommandBuffer cmd, const SunCascades& cascades, std::span<const MeshDraw> meshes) {
        for (std::uint32_t layer{}; layer != maxCascades; ++layer) {
            m_Pass.Begin(cmd, m_Framebuffers[layer]);
            if (static_cast<std::int32_t>(layer) < cascades.data.cascadeCount) {
                vk::Viewport viewport{ VkExtent2D{ mapSize, mapSize }, false };
                viewport.Set(cmd);
                vk::Scissor scissor{ VkExtent2D{ mapSize, mapSize } };
                scissor.Set(cmd);
                m_Pipeline.Bind(cmd);
                vkCmdSetDepthBias(cmd, 0.0f, 0.0f, slopeBias);
                for (const MeshDraw& draw : meshes) {
                    const xmm::Matrix transform{ cascades.viewProjection[layer] * draw.model };
                    vkCmdPushConstants(cmd, m_Layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(transform), &transform);
                    draw.mesh->Bind(cmd);
                    vkCmdDrawIndexed(cmd, draw.mesh->GetIndexCount(), 1u, 0u, 0, 0u);
                }
            }
            m_Pass.End(cmd);
        }
    }

    VkDescriptorImageInfo SunShadows::GetDescriptor() const noexcept {
        return { m_Sampler, m_Maps.GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };
    }
} // namespace adh
