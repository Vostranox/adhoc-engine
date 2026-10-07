#include "TextureCube.hpp"
#include "CommandBuffer.hpp"
#include "Context.hpp"
#include "Memory.hpp"
#include "Tools.hpp"
#include "UniformBuffer.hpp"
#include <Event/Event.hpp>

#include <stb/stb_image.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace adh {
    namespace vk {
        static void GenerateMipMaps(VkImage image, std::uint32_t size, std::uint32_t mipLevels) {
            CommandBuffer commandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, DeviceQueues::Family::eGraphics);
            VkCommandBuffer cmd{ commandBuffer.Begin() };

            auto levelSize{ static_cast<std::int32_t>(size) };
            for (std::uint32_t level{ 1u }; level != mipLevels; ++level) {
                ImageBarrier(cmd, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                             image, VK_IMAGE_ASPECT_COLOR_BIT, level - 1u, 1u, TextureCube::faceCount);

                const std::int32_t nextSize{ std::max(levelSize / 2, 1) };
                VkImageBlit blit{};
                blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level - 1u, 0u, TextureCube::faceCount };
                blit.srcOffsets[1]  = { levelSize, levelSize, 1 };
                blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level, 0u, TextureCube::faceCount };
                blit.dstOffsets[1]  = { nextSize, nextSize, 1 };
                vkCmdBlitImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);

                ImageBarrier(cmd, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                             image, VK_IMAGE_ASPECT_COLOR_BIT, level - 1u, 1u, TextureCube::faceCount);
                levelSize = nextSize;
            }

            ImageBarrier(cmd, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                         image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels - 1u, 1u, TextureCube::faceCount);
            commandBuffer.End();

            auto submitInfo{ initializers::SubmitInfo(1u, &commandBuffer[0]) };
            auto queue{ Context::Get()->GetQueue(DeviceQueues::Family::eGraphics).queue };
            ADH_THROW(vkQueueSubmit(queue, 1u, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
                      "Failed to submit to queue!");
            vkQueueWaitIdle(queue);

            commandBuffer.Free();
        }

        void TextureCube::Create(const std::array<std::string, faceCount>& faceFiles, const Sampler& sampler) {
            stbi_set_flip_vertically_on_load(false);

            const auto maxSize{ tools::GetPhysicalDeviceProperties(Context::Get()->GetPhysicalDevice()).limits.maxImageDimensionCube };
            std::vector<stbi_uc> pixels;
            int size{};
            std::string error;
            for (const auto& file : faceFiles) {
                int width{}, height{}, channels{};
                stbi_uc* face{ stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha) };
                if (!face) {
                    error = "[" + file + "] Failed to load cube map face: " + stbi_failure_reason();
                } else if (width != height || (size != 0 && width != size) || static_cast<std::uint32_t>(width) > maxSize) {
                    error = "[" + file + "] The faces of a cube map must be squares of one size, at most " + std::to_string(maxSize) + " pixels";
                } else {
                    size = width;
                    pixels.insert(pixels.end(), face, face + static_cast<std::size_t>(size) * static_cast<std::size_t>(size) * 4u);
                }
                stbi_image_free(face);
                if (!error.empty()) {
                    break;
                }
            }
            if (!error.empty()) {
                EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, error.c_str());

                constexpr stbi_uc magenta[]{ 255, 0, 255, 255 };
                size = 1;
                pixels.clear();
                for (std::uint32_t face{}; face != faceCount; ++face) {
                    pixels.insert(pixels.end(), std::begin(magenta), std::end(magenta));
                }
            }

            const auto extent{ static_cast<std::uint32_t>(size) };
            const auto mipLevels{ static_cast<std::uint32_t>(std::floor(std::log2(extent))) + 1u };
            UniformBuffer staging{ pixels.data(), pixels.size(), 1u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
            m_Image.Create(
                { extent, extent, 1u },
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_TYPE_2D,
                VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                mipLevels,
                faceCount,
                VK_SAMPLE_COUNT_1_BIT,
                VkImageUsageFlagBits(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_CUBE,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_SHARING_MODE_EXCLUSIVE);

            TransferImageLayout(m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipLevels, faceCount);
            CopyBufferToImage(staging, m_Image, VK_IMAGE_ASPECT_COLOR_BIT, { extent, extent, 1u }, 0u, faceCount);
            GenerateMipMaps(m_Image, extent, mipLevels);

            m_Descriptor = { sampler, m_Image.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        }

        void TextureCube::Destroy() noexcept {
            m_Image.Destroy();
            m_Descriptor = {};
        }

        const VkDescriptorImageInfo& TextureCube::GetDescriptor() const noexcept {
            return m_Descriptor;
        }
    } // namespace vk
} // namespace adh
