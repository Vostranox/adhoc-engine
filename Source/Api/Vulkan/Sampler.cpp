#include "Sampler.hpp"
#include "Context.hpp"
#include "Initializers.hpp"
#include "Tools.hpp"

namespace adh {
    namespace vk {
        Sampler::Sampler() noexcept : m_Sampler{ VK_NULL_HANDLE } {
        }

        Sampler::Sampler(
            VkFilter filter,
            VkSamplerAddressMode mode,
            VkSamplerMipmapMode mipmapMode,
            VkCompareOp compareOp,
            VkBool32 useMipMap,
            VkBool32 anisotropyEnable) ADH_NOEXCEPT {
            Create(filter, mode, mipmapMode, compareOp, useMipMap, anisotropyEnable);
        }

        Sampler::Sampler(Sampler&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Sampler& Sampler::operator=(Sampler&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        Sampler::~Sampler() {
            Clear();
        }

        void Sampler::Create(
            VkFilter filter,
            VkSamplerAddressMode mode,
            VkSamplerMipmapMode mipmapMode,
            VkCompareOp compareOp,
            VkBool32 useMipMap,
            VkBool32 anisotropyEnable) ADH_NOEXCEPT {
            auto physicalDeviceProperties{ tools::GetPhysicalDeviceProperties(Context::Get()->GetPhysicalDevice()) };

            auto info{ initializers::SamplerCreateInfo(
                filter,
                filter,
                mipmapMode,
                mode,
                compareOp,
                useMipMap,
                physicalDeviceProperties.limits.maxSamplerAnisotropy,
                anisotropyEnable) };

            ADH_THROW(vkCreateSampler(Context::Get()->GetDevice(), &info, nullptr, &m_Sampler) == VK_SUCCESS,
                      "Failed to create sampler!");
        }

        VkSampler Sampler::Get() noexcept {
            return m_Sampler;
        }

        VkSampler Sampler::Get() const noexcept {
            return m_Sampler;
        }

        void Sampler::Destroy() noexcept {
            Clear();
        }

        Sampler::operator VkSampler() noexcept {
            return m_Sampler;
        }

        Sampler::operator VkSampler() const noexcept {
            return m_Sampler;
        }

        void Sampler::MoveConstruct(Sampler&& rhs) noexcept {
            m_Sampler     = rhs.m_Sampler;
            rhs.m_Sampler = VK_NULL_HANDLE;
        }

        void Sampler::Clear() noexcept {
            if (m_Sampler != VK_NULL_HANDLE) {
                vkDestroySampler(Context::Get()->GetDevice(), m_Sampler, nullptr);
                m_Sampler = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
