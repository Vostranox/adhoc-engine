#pragma once
#include <Utility.hpp>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Sampler {
          public:
            Sampler() noexcept;

            Sampler(
                VkFilter filter,
                VkSamplerAddressMode mode,
                VkSamplerMipmapMode mipmapMode,
                VkCompareOp compareOp,
                VkBool32 useMipMap        = VK_FALSE,
                VkBool32 anisotropyEnable = VK_FALSE) ADH_NOEXCEPT;

            Sampler(const Sampler& rhs) = delete;

            Sampler& operator=(const Sampler& rhs) = delete;

            Sampler(Sampler&& rhs) noexcept;

            Sampler& operator=(Sampler&& rhs) noexcept;

            ~Sampler();

            void Create(
                VkFilter filter,
                VkSamplerAddressMode mode,
                VkSamplerMipmapMode mipmapMode,
                VkCompareOp compareOp,
                VkBool32 useMipMap        = VK_FALSE,
                VkBool32 anisotropyEnable = VK_FALSE) ADH_NOEXCEPT;

            VkSampler Get() noexcept;

            VkSampler Get() const noexcept;

            void Destroy() noexcept;

            operator VkSampler() noexcept;

            operator VkSampler() const noexcept;

          private:
            void MoveConstruct(Sampler&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkSampler m_Sampler;
        };
    } // namespace vk
} // namespace adh
