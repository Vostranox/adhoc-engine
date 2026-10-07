#include "Allocator.hpp"
#include "Context.hpp"
#include "Initializers.hpp"
#include "Tools.hpp"

#include <cstring>

namespace adh {
    namespace vk {
        static void FreeRange(const MemoryData& memoryData) {
            auto& freeBlocks{ memoryData.memoryBlock->freeBlocks };
            freeBlocks.EmplaceBack(memoryData.head, memoryData.tail);
            freeBlocks.Sort();

            std::size_t last{};
            for (std::size_t i{ 1u }; i != freeBlocks.GetSize(); ++i) {
                if (freeBlocks[last].tail == freeBlocks[i].head) {
                    freeBlocks[last].tail = freeBlocks[i].tail;
                } else {
                    freeBlocks[++last] = freeBlocks[i];
                }
            }
            while (freeBlocks.GetSize() > last + 1u) {
                freeBlocks.PopBack();
            }
        }

        static bool IsUnused(const MemoryBlock& block) noexcept {
            return block.freeBlocks.GetSize() == 1u && block.freeBlocks[0].head == 0u && block.freeBlocks[0].tail == block.size;
        }

        void Allocator::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryPropertyFlag, BufferData& bufferData) {
            GetInstance().CreateBuffer2(size, usage, memoryPropertyFlag, bufferData);
        }

        void Allocator::CreateImage(
            VkExtent3D extent,
            VkFormat format,
            VkImageTiling imageTiling,
            VkImageType imageType,
            VkImageCreateFlagBits imageCreateFlags,
            std::uint32_t mipLevels,
            std::uint32_t arrayLayers,
            VkSampleCountFlagBits samples,
            VkImageUsageFlagBits imageUsageFlag,
            VkMemoryPropertyFlagBits memoryPropertyFlag,
            VkSharingMode sharingMode,
            ImageData& imageData) {
            GetInstance().CreateImage2(
                extent,
                format,
                imageTiling,
                imageType,
                imageCreateFlags,
                mipLevels,
                arrayLayers,
                samples,
                imageUsageFlag,
                memoryPropertyFlag,
                sharingMode,
                imageData);
        }

        void Allocator::CreateImageView(
            VkImageViewType imageViewType,
            VkFormat format,
            VkImageAspectFlagBits imageAspectFlag,
            std::uint32_t levelCount,
            std::uint32_t layerCount,
            ImageData& imageData) {
            GetInstance().CreateImageView2(
                imageViewType,
                format,
                imageAspectFlag,
                levelCount,
                layerCount,
                imageData);
        }

        void Allocator::BindImage(ImageData& imageData) ADH_NOEXCEPT {
            GetInstance().BindImageMemory(imageData);
        }

        void Allocator::BindBuffer(BufferData& bufferData) ADH_NOEXCEPT {
            GetInstance().BindBufferMemory(bufferData);
        }

        void Allocator::Map(const void* data, VkDeviceSize size, const MemoryData& memoryData, void*& toMap) ADH_NOEXCEPT {
            GetInstance().MapMemory(data, size, memoryData, toMap);
        }

        void Allocator::Flush(VkDeviceSize size, VkDeviceSize offset, VkDeviceMemory deviceMemory) ADH_NOEXCEPT {
            GetInstance().FlushMemory(size, offset, deviceMemory);
        }

        void Allocator::DestroyBuffer(BufferData& bufferData) noexcept {
            GetInstance().DestroyBuffer2(bufferData);
        }

        void Allocator::DestroyImage(ImageData& imageData) noexcept {
            GetInstance().DestroyImage2(imageData);
        }

        void Allocator::Flush() noexcept {
            GetInstance().Flush2();
        }

        void Allocator::Destroy() noexcept {
            GetInstance().Clear();
        }

        Allocator& Allocator::GetInstance() noexcept {
            static Allocator allocator;
            return allocator;
        }

        void Allocator::CreateBuffer2(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryPropertyFlag, BufferData& bufferData) {
            auto* context{ Context::Get() };
            auto info{ initializers::BufferCreateInfo(size, usage) };
            ADH_THROW(vkCreateBuffer(context->GetDevice(), &info, nullptr, &bufferData.buffer) == VK_SUCCESS,
                      "Failed to create buffer!");

            auto memoryRequirements{ tools::GetBufferMemoryRequirements(context->GetDevice(), bufferData.buffer) };
            auto memoryTypeIndex{ tools::GetMemoryTypeIndex(context->GetPhysicalDevice(), memoryRequirements, memoryPropertyFlag) };

            Emplace(memoryRequirements, memoryTypeIndex, bufferData.memoryData);
        }

        void Allocator::CreateImage2(
            VkExtent3D extent,
            VkFormat format,
            VkImageTiling imageTiling,
            VkImageType imageType,
            VkImageCreateFlagBits imageCreateFlags,
            std::uint32_t mipLevels,
            std::uint32_t arrayLayers,
            VkSampleCountFlagBits samples,
            VkImageUsageFlagBits imageUsageFlag,
            VkMemoryPropertyFlagBits memoryPropertyFlag,
            VkSharingMode sharingMode,
            ImageData& imageData) {
            auto info{
                initializers::ImageCreateInfo(
                    extent,
                    format,
                    imageTiling,
                    imageType,
                    imageCreateFlags,
                    mipLevels,
                    arrayLayers,
                    samples,
                    imageUsageFlag,
                    sharingMode)
            };

            auto* context{ Context::Get() };
            ADH_THROW(vkCreateImage(context->GetDevice(), &info, nullptr, &imageData.image) == VK_SUCCESS,
                      "Failed to create image!");
            auto memoryRequirements{ tools::GetImageMemoryRequirements(context->GetDevice(), imageData.image) };
            auto memoryTypeIndex{ tools::GetMemoryTypeIndex(context->GetPhysicalDevice(), memoryRequirements, memoryPropertyFlag) };

            Emplace(memoryRequirements, memoryTypeIndex, imageData.memoryData);
        }

        void Allocator::CreateImageView2(
            VkImageViewType imageViewType,
            VkFormat format,
            VkImageAspectFlagBits imageAspectFlag,
            std::uint32_t levelCount,
            std::uint32_t layerCount,
            ImageData& imageData) ADH_NOEXCEPT {
            auto imageViewInfo{
                initializers::ImageViewCreateInfo(
                    imageData.image,
                    imageViewType,
                    format,
                    imageAspectFlag,
                    levelCount,
                    layerCount)
            };

            ADH_THROW(vkCreateImageView(Context::Get()->GetDevice(), &imageViewInfo, nullptr, &imageData.imageView) == VK_SUCCESS,
                      "Failed to create image view!");
        }

        void Allocator::MapMemory(const void* data, VkDeviceSize size, const MemoryData& memoryData, void*& toMap) ADH_NOEXCEPT {
            ADH_THROW(memoryData.memoryBlock->mappedMemory, "Mapping memory that isn't host visible!");
            toMap = static_cast<char*>(memoryData.memoryBlock->mappedMemory) + memoryData.head;
            if (data) {
                std::memcpy(toMap, data, static_cast<std::size_t>(size));
            }
        }

        void Allocator::FlushMemory(VkDeviceSize size, VkDeviceSize offset, VkDeviceMemory deviceMemory) ADH_NOEXCEPT {
            auto mappedRange{ initializers::MappedMemoryRange(size, offset, deviceMemory) };
            ADH_THROW(vkFlushMappedMemoryRanges(Context::Get()->GetDevice(), 1, &mappedRange) == VK_SUCCESS,
                      "Failed to flush memory!");
        }

        void Allocator::BindBufferMemory(BufferData& bufferData) ADH_NOEXCEPT {
            ADH_THROW(vkBindBufferMemory(
                          Context::Get()->GetDevice(),
                          bufferData.buffer,
                          bufferData.memoryData.memoryBlock->deviceMemory,
                          bufferData.memoryData.head) == VK_SUCCESS,
                      "Faield to bind buffer memory!");
        }

        void Allocator::BindImageMemory(ImageData& imageData) ADH_NOEXCEPT {
            ADH_THROW(vkBindImageMemory(
                          Context::Get()->GetDevice(),
                          imageData.image,
                          imageData.memoryData.memoryBlock->deviceMemory,
                          imageData.memoryData.head) == VK_SUCCESS,
                      "Failed to bind image memory!");
        }

        VkDeviceSize Allocator::GetAlignedMemory(VkDeviceSize size, VkDeviceSize alignment) noexcept {
            return (size + alignment - 1u) & ~(alignment - 1u);
        }

        void Allocator::Emplace(VkMemoryRequirements memoryRequirements, std::uint32_t memoryTypeIndex, MemoryData& memoryData) {
            bool allocated  = false;
            auto bufferSize = GetAlignedMemory(memoryRequirements.size, memoryRequirements.alignment);

            MemoryType memoryType{
                .alignment = memoryRequirements.alignment,
                .typeBits  = memoryRequirements.memoryTypeBits,
                .typeIndex = memoryTypeIndex
            };

            auto res    = m_Blocks.emplace(memoryType, List<MemoryBlock>{});
            auto& block = res.first->second;

            auto allocation = [&]() {
                auto blockSize = m_BlockSize >= bufferSize ? m_BlockSize : GetAlignedMemory(bufferSize, memoryRequirements.alignment);

                auto newBlock  = block.EmplaceBack();
                newBlock->size = blockSize;
                newBlock->freeBlocks.EmplaceBack(bufferSize, blockSize);

                auto info{ initializers::MemoryAllocateInfo(blockSize, memoryTypeIndex) };
                ADH_THROW(vkAllocateMemory(Context::Get()->GetDevice(), &info, nullptr, &newBlock->deviceMemory) == VK_SUCCESS,
                          "Failed to allocate memory!");

                auto memoryProperties{ tools::GetPhysicalDeviceMemoryProperties(Context::Get()->GetPhysicalDevice()) };
                if (memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                    ADH_THROW(vkMapMemory(Context::Get()->GetDevice(), newBlock->deviceMemory, 0u, VK_WHOLE_SIZE, 0u, &newBlock->mappedMemory) == VK_SUCCESS,
                              "Failed to map memory!");
                }

                memoryData.memoryBlock = &(*newBlock);
                memoryData.head        = 0;
                memoryData.tail        = bufferSize;
            };

            if (res.second) {
                allocation();
                return;
            }

            for (auto&& i : block) {
                for (std::size_t j{}; j != i.freeBlocks.GetSize(); ++j) {
                    if ((i.freeBlocks[j].tail - i.freeBlocks[j].head) >= bufferSize) {
                        memoryData.memoryBlock = &i;
                        memoryData.head        = i.freeBlocks[j].head;
                        memoryData.tail        = i.freeBlocks[j].head + bufferSize;
                        i.freeBlocks[j].head += bufferSize;

                        if (i.freeBlocks[j].head == i.freeBlocks[j].tail) {
                            i.freeBlocks[j] = i.freeBlocks[i.freeBlocks.GetSize() - 1];
                            i.freeBlocks.PopBack();
                            i.freeBlocks.Sort();
                        }
                        allocated = true;
                        break;
                    }
                }
                if (allocated) {
                    break;
                }
            }
            if (!allocated) {
                allocation();
            }
        }

        void Allocator::DestroyBuffer2(BufferData& bufferData) noexcept {
            m_DestroyQueue.EmplaceBack([=]() {
                FreeRange(bufferData.memoryData);
                vkDestroyBuffer(Context::Get()->GetDevice(), bufferData.buffer, nullptr);
            });
        }

        void Allocator::DestroyImage2(ImageData& imageData) noexcept {
            m_DestroyQueue.EmplaceBack([=]() {
                FreeRange(imageData.memoryData);

                auto device{ Context::Get()->GetDevice() };
                vkDestroyImage(device, imageData.image, nullptr);
                vkDestroyImageView(device, imageData.imageView, nullptr);
            });
        }

        void Allocator::Flush2() noexcept {
            if (m_DestroyQueue.IsEmpty()) {
                return;
            }

            auto device{ Context::Get()->GetDevice() };
            vkDeviceWaitIdle(device);
            for (auto&& i : m_DestroyQueue) {
                i();
            }
            m_DestroyQueue.Clear();

            for (auto&& [memoryType, blocks] : m_Blocks) {
                bool hasSpare{ false };
                for (auto i{ blocks.begin() }; i != blocks.end();) {
                    if (IsUnused(*i) && hasSpare) {
                        vkFreeMemory(device, i->deviceMemory, nullptr);
                        i = blocks.Erase(i);
                    } else {
                        hasSpare = hasSpare || IsUnused(*i);
                        ++i;
                    }
                }
            }
        }

        void Allocator::Clear() noexcept {
            Flush2();
            if (!m_Blocks.empty()) {
                auto device{ Context::Get()->GetDevice() };
                vkDeviceWaitIdle(device);
                for (auto&& i : m_Blocks) {
                    for (auto&& j : i.second) {
                        vkFreeMemory(device, j.deviceMemory, nullptr);
                    }
                }
            }
        }
    } // namespace vk
} // namespace adh
