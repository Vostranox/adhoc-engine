#pragma once
#include "Image.hpp"
#include "Sampler.hpp"
#include "UniformBuffer.hpp"

#include <Std/Array.hpp>

#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Texture2D {
          public:
            Texture2D() noexcept;

            Texture2D(
                const char* filePath,
                VkImageUsageFlagBits imageUsage,
                const Sampler* sampler,
                VkBool32 generateMinMap   = VK_FALSE,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

            Texture2D(
                const void* data,
                std::size_t size,
                VkExtent2D extent,
                VkImageUsageFlagBits imageUsage,
                const Sampler* sampler,
                VkBool32 generateMinMap   = VK_FALSE,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

            Texture2D(
                VkExtent2D extent,
                VkImageUsageFlagBits imageUsage,
                VkImageLayout imageLayout,
                const Sampler* sampler,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

            Texture2D(
                const char* filePath,
                VkImageUsageFlagBits imageUsage,
                VkFilter filter,
                VkBool32 generateMinMap   = VK_FALSE,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

            Texture2D(const Texture2D& rhs) = delete;

            Texture2D& operator=(const Texture2D& rhs) = delete;

            Texture2D(Texture2D&& rhs) noexcept;

            Texture2D& operator=(Texture2D&& rhs) noexcept;

            ~Texture2D() = default;

            void Create(
                const char* filePath,
                VkImageUsageFlagBits imageUsage,
                const Sampler* sampler,
                VkBool32 generateMinMap   = VK_FALSE,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                VkFormat format           = VK_FORMAT_R8G8B8A8_UNORM);

            void Create(
                const void* data,
                std::size_t size,
                VkExtent2D extent,
                VkImageUsageFlagBits imageUsage,
                const Sampler* sampler,
                VkBool32 generateMinMap   = VK_FALSE,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                VkFormat format           = VK_FORMAT_R8G8B8A8_UNORM);

            void Create(
                VkExtent2D extent,
                VkImageUsageFlagBits imageUsage,
                VkImageLayout imageLayout,
                const Sampler* sampler,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

            void Create(
                const char* filePath,
                VkImageUsageFlagBits imageUsage,
                VkFilter filter,
                VkBool32 generateMinMap   = VK_FALSE,
                VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                VkFormat format           = VK_FORMAT_R8G8B8A8_UNORM);

            VkImage GetImage() noexcept;

            VkImage GetImage() const noexcept;

            VkImageView GetImageView() noexcept;

            VkImageView GetImageView() const noexcept;

            std::size_t GetWidth() const noexcept;

            std::size_t GetHeight() const noexcept;

            VkDescriptorImageInfo GetDescriptor() noexcept;

            const VkDescriptorImageInfo GetDescriptor() const noexcept;

            void SetLayout(VkImageLayout imageLayout) noexcept;

            std::uint32_t GetMipLevels() const noexcept;

            bool IsOpaque() const noexcept;

            void Destroy() noexcept;

            operator VkImageView() noexcept;

            operator VkImageView() const noexcept;

            static void InitializeDefaultSamplers();

            static void CleanUpDefaultSamplers();

            static const Sampler* GetDefaultSampler(VkFilter filter) noexcept;

          private:
            VkImageUsageFlagBits SelectImageUsage(VkImageUsageFlagBits imageUsage, VkBool32 generateMinMap) noexcept;

            void SelectImageLayout(VkImageUsageFlagBits imageUsage) noexcept;

            void CreateImage(
                const UniformBuffer& staging,
                VkFormat format,
                VkBool32 generateMinMap,
                VkImageUsageFlagBits usageFlag,
                VkSharingMode sharingMode);

            void CreateImage(
                VkImageUsageFlagBits usageFlag,
                VkImageLayout imageLayout,
                VkSharingMode sharingMode);

            void GenerateMipMaps() ADH_NOEXCEPT;

            void GenerateImageBarriers(VkCommandBuffer commandBuffer, std::int32_t mipWidth, std::int32_t mipHeight) noexcept;

            void BlitImage(VkCommandBuffer commandBuffer, std::int32_t index, std::int32_t mipWidth, std::int32_t mipHeight) noexcept;

            void InitializeDescriptor(const Sampler* sampler) noexcept;

            void MoveConstruct(Texture2D&& rhs) noexcept;

            void Clear() noexcept;

          private:
            Image m_Image;
            VkDescriptorImageInfo m_Descriptor;
            VkExtent2D m_Extent;
            std::uint32_t m_MipLevels;
            bool m_IsOpaque{ true };

            inline static Array<Sampler> m_DefaultSamplers;
        };
    } // namespace vk
} // namespace adh
