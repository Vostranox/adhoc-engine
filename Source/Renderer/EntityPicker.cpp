#include "EntityPicker.hpp"

#include <Vulkan/Initializers.hpp>

#include <algorithm>
#include <cstring>

namespace adh {
    static std::uint32_t Texel(float t, std::uint32_t size) noexcept {
        return std::min(static_cast<std::uint32_t>(std::max(t, 0.0f) * static_cast<float>(size)), size - 1u);
    }

    void EntityPicker::Create() {
        m_Buffer.Create(sizeof(std::uint32_t[2]), 1u, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        m_Buffer.Map(nullptr, m_Texel);
    }

    void EntityPicker::Record(VkCommandBuffer cmd, const SceneView& view, float u, float v) {
        const std::uint32_t x{ Texel(u, view.extent.width) };
        const std::uint32_t y{ view.extent.height - 1u - Texel(v, view.extent.height) };
        auto copy{ vk::initializers::BufferImageCopy(VK_IMAGE_ASPECT_COLOR_BIT, { 1u, 1u, 1u }, 0u, 1u) };
        copy.imageOffset = { static_cast<std::int32_t>(x), static_cast<std::int32_t>(y), 0 };
        vkCmdCopyImageToBuffer(cmd, view.entityIds.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Buffer, 1u, &copy);

        const auto barrier{ vk::initializers::BufferMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT,
                                                                  VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_Buffer, 0u, VK_WHOLE_SIZE) };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0u, 0u, nullptr, 1u, &barrier, 0u, nullptr);

        m_IsPending = true;
    }

    bool EntityPicker::IsPending() const noexcept {
        return m_IsPending;
    }

    ecs::Entity EntityPicker::Read() noexcept {
        m_IsPending = false;
        std::uint32_t words[2];
        std::memcpy(words, m_Texel, sizeof(words));
        const std::uint64_t id{ words[0] | (static_cast<std::uint64_t>(words[1]) << 32u) };
        return id == 0u ? ecs::NULL_ENTITY : ecs::Entity{ id };
    }
} // namespace adh
