#pragma once
#include "Image.hpp"
#include "Sampler.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <string>

namespace adh {
    namespace vk {
        class TextureCube {
          public:
            static constexpr std::uint32_t faceCount{ 6u };

            void Create(const std::array<std::string, faceCount>& faceFiles, const Sampler& sampler);

            void Destroy() noexcept;

            const VkDescriptorImageInfo& GetDescriptor() const noexcept;

          private:
            Image m_Image;
            VkDescriptorImageInfo m_Descriptor{};
        };
    } // namespace vk
} // namespace adh
