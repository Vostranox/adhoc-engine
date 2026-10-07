#pragma once
#include <Vulkan/Texture2D.hpp>

#include <array>
#include <cstdint>
#include <string>

namespace adh {
    class MaterialTextures {
      public:
        enum Map : std::uint32_t {
            eAlbedo,
            eNormal,
            eHeight,
            eMapCount
        };

        static constexpr const char* mapNames[eMapCount]{ "albedo", "normal", "height" };

        MaterialTextures() = default;

        MaterialTextures(const MaterialTextures&) = delete;

        MaterialTextures& operator=(const MaterialTextures&) = delete;

        MaterialTextures(MaterialTextures&& rhs) noexcept;

        MaterialTextures& operator=(MaterialTextures&& rhs) noexcept;

        ~MaterialTextures();

        void Load();

        VkDescriptorSet GetDescriptorSet() const noexcept;

        bool IsOpaque() const noexcept;

        static void CreateDefaults();

        static void DestroyDefaults() noexcept;

        static VkDescriptorSet GetDefaultDescriptorSet() noexcept;

      private:
        void MoveConstruct(MaterialTextures&& rhs) noexcept;

        void FreeDescriptorSet() noexcept;

      public:
        bool enabled{ true };
        std::array<std::string, eMapCount> files;

      private:
        static constexpr std::uint32_t noDescriptorSet{ ~0u };
        std::array<vk::Texture2D, eMapCount> m_Maps;
        std::uint32_t m_DescriptorSetId{ noDescriptorSet };
        inline static std::array<vk::Texture2D, eMapCount> m_DefaultMaps;
        inline static std::uint32_t m_DefaultDescriptorSetId{ noDescriptorSet };
    };
} // namespace adh
