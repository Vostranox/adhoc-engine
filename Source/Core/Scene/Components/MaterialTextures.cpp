#include "MaterialTextures.hpp"

#include <Vulkan/Context.hpp>
#include <Vulkan/DescriptorSet.hpp>

#include <utility>

namespace adh {
    static constexpr VkFormat mapFormats[MaterialTextures::eMapCount]{ VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM };

    MaterialTextures::MaterialTextures(MaterialTextures&& rhs) noexcept {
        MoveConstruct(std::move(rhs));
    }

    MaterialTextures& MaterialTextures::operator=(MaterialTextures&& rhs) noexcept {
        FreeDescriptorSet();
        MoveConstruct(std::move(rhs));

        return *this;
    }

    MaterialTextures::~MaterialTextures() {
        FreeDescriptorSet();
    }

    void MaterialTextures::Load() {
        FreeDescriptorSet();
        std::array<VkDescriptorImageInfo, eMapCount> descriptors;
        for (std::uint32_t map{}; map != eMapCount; ++map) {
            m_Maps[map].Destroy();
            if (files[map].empty()) {
                descriptors[map] = m_DefaultMaps[map].GetDescriptor();
            } else {
                const std::string filePath{ vk::Context::Get()->GetDataDirectory() + "Assets/Textures/" + files[map] };
                m_Maps[map].Create(filePath.data(), VK_IMAGE_USAGE_SAMPLED_BIT, VK_FILTER_LINEAR, VK_TRUE, VK_SHARING_MODE_EXCLUSIVE, mapFormats[map]);
                descriptors[map] = m_Maps[map].GetDescriptor();
            }
        }
        m_DescriptorSetId = vk::TextureDescriptors::GetDescriptorID(descriptors);
    }

    VkDescriptorSet MaterialTextures::GetDescriptorSet() const noexcept {
        return vk::TextureDescriptors::GetDescriptor(enabled && m_DescriptorSetId != noDescriptorSet ? m_DescriptorSetId : m_DefaultDescriptorSetId);
    }

    bool MaterialTextures::IsOpaque() const noexcept {
        return !enabled || m_Maps[eAlbedo].IsOpaque();
    }

    void MaterialTextures::CreateDefaults() {
        const std::uint8_t white[]{ 255u, 255u, 255u, 255u };
        const std::uint16_t flatNormal[]{ 0x3800u, 0x3800u, 0x3C00u, 0x3C00u };
        const vk::Sampler* sampler{ vk::Texture2D::GetDefaultSampler(VK_FILTER_LINEAR) };
        m_DefaultMaps[eAlbedo].Create(white, sizeof(white), { 1u, 1u }, VK_IMAGE_USAGE_SAMPLED_BIT, sampler, VK_FALSE, VK_SHARING_MODE_EXCLUSIVE, VK_FORMAT_R8G8B8A8_SRGB);
        m_DefaultMaps[eNormal].Create(flatNormal, sizeof(flatNormal), { 1u, 1u }, VK_IMAGE_USAGE_SAMPLED_BIT, sampler, VK_FALSE, VK_SHARING_MODE_EXCLUSIVE, VK_FORMAT_R16G16B16A16_SFLOAT);
        m_DefaultMaps[eHeight].Create(white, sizeof(white), { 1u, 1u }, VK_IMAGE_USAGE_SAMPLED_BIT, sampler, VK_FALSE, VK_SHARING_MODE_EXCLUSIVE, VK_FORMAT_R8G8B8A8_UNORM);

        std::array<VkDescriptorImageInfo, eMapCount> descriptors;
        for (std::uint32_t map{}; map != eMapCount; ++map) {
            descriptors[map] = m_DefaultMaps[map].GetDescriptor();
        }
        m_DefaultDescriptorSetId = vk::TextureDescriptors::GetDescriptorID(descriptors);
    }

    void MaterialTextures::DestroyDefaults() noexcept {
        for (auto& map : m_DefaultMaps) {
            map.Destroy();
        }
        if (m_DefaultDescriptorSetId != noDescriptorSet) {
            vk::TextureDescriptors::FreeDescriptor(m_DefaultDescriptorSetId);
            m_DefaultDescriptorSetId = noDescriptorSet;
        }
    }

    VkDescriptorSet MaterialTextures::GetDefaultDescriptorSet() noexcept {
        return vk::TextureDescriptors::GetDescriptor(m_DefaultDescriptorSetId);
    }

    void MaterialTextures::MoveConstruct(MaterialTextures&& rhs) noexcept {
        enabled               = rhs.enabled;
        files                 = std::move(rhs.files);
        m_Maps                = std::move(rhs.m_Maps);
        m_DescriptorSetId     = rhs.m_DescriptorSetId;
        rhs.m_DescriptorSetId = noDescriptorSet;
    }

    void MaterialTextures::FreeDescriptorSet() noexcept {
        if (m_DescriptorSetId != noDescriptorSet) {
            vk::TextureDescriptors::FreeDescriptor(m_DescriptorSetId);
            m_DescriptorSetId = noDescriptorSet;
        }
    }
} // namespace adh
