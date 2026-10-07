#pragma once
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Context.hpp"
#include "Initializers.hpp"
#include <Utility.hpp>

#include <iostream>
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        inline void CopyBuffer(VkBuffer src, VkBuffer dst, std::uint64_t size) {
            CommandBuffer commandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, DeviceQueues::Family::eTransfer);
            commandBuffer.Begin();
            auto bufferCopy{ initializers::BufferCopy(size) };
            vkCmdCopyBuffer(commandBuffer[0], src, dst, 1u, &bufferCopy);
            commandBuffer.End();

            auto submitInfo{ initializers::SubmitInfo(
                1u, &commandBuffer[0],
                0, nullptr, nullptr,
                0, nullptr) };
            auto queue{ Context::Get()->GetQueue(DeviceQueues::Family::eTransfer).queue };
            ADH_THROW(vkQueueSubmit(queue, 1u, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
                      "Failed to submit to queue!");
            vkQueueWaitIdle(queue);

            commandBuffer.Free();
        }

        inline void CopyBufferToImage(
            VkBuffer srcBuffer,
            VkImage dstImage,
            VkImageAspectFlagBits aspectFlag,
            VkExtent3D extent,
            std::uint32_t mipLevel   = 0u,
            std::uint32_t layerCount = 1u) {
            CommandBuffer commandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, DeviceQueues::Family::eTransfer);
            commandBuffer.Begin();

            auto imageCopy{ initializers::BufferImageCopy(aspectFlag, extent, mipLevel, layerCount) };
            vkCmdCopyBufferToImage(
                commandBuffer[0],
                srcBuffer,
                dstImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1u,
                &imageCopy);

            commandBuffer.End();

            auto submitInfo{ initializers::SubmitInfo(
                1u, &commandBuffer[0],
                0, nullptr, nullptr,
                0, nullptr) };
            auto queue{ Context::Get()->GetQueue(DeviceQueues::Family::eTransfer).queue };
            ADH_THROW(vkQueueSubmit(queue, 1u, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
                      "Failed to submit to queue!");
            vkQueueWaitIdle(queue);

            commandBuffer.Free();
        }

        inline void TransferImageLayout(
            VkImage image,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            VkImageAspectFlagBits aspectFlag,
            VkPipelineStageFlagBits srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlagBits dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            std::uint32_t levelCount             = 1u,
            std::uint32_t layerCount             = 1u) {
            CommandBuffer commandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, DeviceQueues::Family::eTransfer);
            commandBuffer.Begin();

            auto barrier{ initializers::ImageMemoryBarrier(
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                oldLayout,
                newLayout,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                image, //
                aspectFlag,
                0u,
                levelCount,
                layerCount) };

            switch (oldLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:

                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;

            default:
                break;
            }

            switch (newLayout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;

            default:
                break;
            }

            vkCmdPipelineBarrier(
                commandBuffer[0],
                srcStageMask,
                dstStageMask,
                0u,
                0u, nullptr,
                0u, nullptr,
                1u, &barrier);

            commandBuffer.End();

            auto submitInfo{ initializers::SubmitInfo(
                1u, &commandBuffer[0],
                0, nullptr, nullptr,
                0, nullptr) };
            auto queue{ Context::Get()->GetQueue(DeviceQueues::Family::eTransfer).queue };
            ADH_THROW(vkQueueSubmit(queue, 1u, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
                      "Failed to submit to queue!");
            vkQueueWaitIdle(queue);

            commandBuffer.Free();
        }

        inline void BufferBarrier(
            VkCommandBuffer commandBuffer,
            VkAccessFlagBits srcAccessMask,
            VkAccessFlagBits dstAccessMask,
            VkPipelineStageFlagBits srcStageMask,
            VkPipelineStageFlagBits dstStageMask,
            std::uint32_t srcQueueFamilyIndex,
            std::uint32_t dstQueueFamilyIndex,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize size) noexcept {
            auto barrier{ initializers::BufferMemoryBarrier(
                srcAccessMask,
                dstAccessMask,
                srcQueueFamilyIndex,
                dstQueueFamilyIndex,
                buffer,
                offset,
                size) };

            vkCmdPipelineBarrier(
                commandBuffer,
                srcStageMask,
                dstStageMask,
                0u,
                0u,
                nullptr,
                1u,
                &barrier,
                0u,
                nullptr);
        }

        inline void ImageBarrier(
            VkCommandBuffer commandBuffer,
            VkAccessFlagBits srcAccessMask,
            VkAccessFlagBits dstAccessMask,
            VkPipelineStageFlagBits srcStageMask,
            VkPipelineStageFlagBits dstStageMask,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            std::uint32_t srcQueueFamilyIndex,
            std::uint32_t dstQueueFamilyIndex,
            VkImage image,
            VkImageAspectFlagBits aspectFlag,
            std::uint32_t baseMipLevel,
            std::uint32_t levelCount = 1u,
            std::uint32_t layerCount = 1u) noexcept {
            auto barrier{ initializers::ImageMemoryBarrier(
                srcAccessMask,
                dstAccessMask,
                oldLayout,
                newLayout,
                srcQueueFamilyIndex,
                dstQueueFamilyIndex,
                image,
                aspectFlag,
                baseMipLevel,
                levelCount,
                layerCount) };

            vkCmdPipelineBarrier(
                commandBuffer,
                srcStageMask,
                dstStageMask,
                0u,
                0u,
                nullptr,
                0u,
                nullptr,
                1u,
                &barrier);
        }

        inline void MemoryBarrier(
            VkCommandBuffer commandBuffer,
            VkAccessFlagBits srcAccessMask,
            VkAccessFlagBits dstAccessMask,
            VkPipelineStageFlagBits srcStageMask,
            VkPipelineStageFlagBits dstStageMask) noexcept {
            auto barrier{ initializers::MemoryBarrier(srcAccessMask, dstAccessMask) };

            vkCmdPipelineBarrier(
                commandBuffer,
                srcStageMask,
                dstStageMask,
                0,
                1u,
                &barrier,
                0u,
                nullptr,
                0u,
                nullptr);
        }
    } // namespace vk
} // namespace adh
