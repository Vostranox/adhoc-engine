#pragma once
#include <Std/Array.hpp>
#include <array>
#include <vector>
#include <vulkan/vulkan.h>

#include <Vulkan/Context.hpp>

#include <Utility.hpp>
#include <Vulkan/Initializers.hpp>
#include <Vulkan/UniformBuffer.hpp>

namespace adh {
    namespace vk {
        class TextureDescriptors {
          public:
            static constexpr uint32_t mapCount{ 3u };
            static constexpr uint32_t setsPerPool{ 1000u };

            static void Initialize() {
                VkDescriptorSetLayoutBinding bindings[mapCount]{};
                for (uint32_t i{}; i != mapCount; ++i) {
                    bindings[i].binding         = i;
                    bindings[i].descriptorCount = 1;
                    bindings[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    bindings[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = mapCount;
                layoutInfo.pBindings    = bindings;

                ADH_THROW(vkCreateDescriptorSetLayout(Context::Get()->GetDevice(), &layoutInfo, nullptr, &setLayout) == VK_SUCCESS,
                          "Failed to create set layout!");

                AddPool();
            }

            static uint32_t GetDescriptorID(const std::array<VkDescriptorImageInfo, mapCount>& maps) {
                auto pool{ mPools.begin() };
                while (pool != mPools.end() && pool->setCount == setsPerPool) {
                    ++pool;
                }
                if (pool == mPools.end()) {
                    AddPool();
                    pool = mPools.end() - 1;
                }

                VkDescriptorSetAllocateInfo info{};
                info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                info.descriptorPool     = pool->handle;
                info.descriptorSetCount = 1;
                info.pSetLayouts        = &setLayout;

                VkDescriptorSet set{ VK_NULL_HANDLE };
                ADH_THROW(vkAllocateDescriptorSets(Context::Get()->GetDevice(), &info, &set) == VK_SUCCESS,
                          "Failed to allocate descriptor sets!");
                ++pool->setCount;

                uint32_t id;
                if (!mFreeIDs.empty()) {
                    id = mFreeIDs[mFreeIDs.size() - 1];
                    mFreeIDs.pop_back();
                    mDescriptorSets[id] = set;
                    mSetPools[id]       = pool->handle;
                } else {
                    id = static_cast<uint32_t>(mDescriptorSets.GetSize());
                    mDescriptorSets.EmplaceBack(set);
                    mSetPools.emplace_back(pool->handle);
                }

                VkWriteDescriptorSet writeSets[mapCount];
                for (uint32_t i{}; i != mapCount; ++i) {
                    writeSets[i] = initializers::WriteDescriptorSet(
                        set,
                        i,
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        &maps[i],
                        nullptr,
                        nullptr);
                }

                vkUpdateDescriptorSets(Context::Get()->GetDevice(), mapCount, writeSets, 0, nullptr);

                return id;
            }

            static VkDescriptorSet GetDescriptor(uint32_t index) {
                return mDescriptorSets[index];
            }

            static void FreeDescriptor(uint32_t id) {
                clearDescriptors.EmplaceBack(id);
            }

            static void Flush() {
                if (!clearDescriptors.IsEmpty()) {
                    auto device{ Context::Get()->GetDevice() };
                    vkDeviceWaitIdle(device);
                    for (std::size_t i{}; i != clearDescriptors.GetSize(); ++i) {
                        auto id{ clearDescriptors[i] };
                        auto pool{ mPools.begin() };
                        while (pool->handle != mSetPools[id]) {
                            ++pool;
                        }
                        vkFreeDescriptorSets(device, pool->handle, 1, &mDescriptorSets[id]);
                        mDescriptorSets[id] = VK_NULL_HANDLE;
                        mFreeIDs.emplace_back(id);
                        if (--pool->setCount == 0 && mPools.size() > 1) {
                            vkDestroyDescriptorPool(device, pool->handle, nullptr);
                            mPools.erase(pool);
                        }
                    }
                    clearDescriptors.Clear();
                }
            }

            static void CleanUp() {
                auto device{ Context::Get()->GetDevice() };
                vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
                for (const Pool& pool : mPools) {
                    vkDestroyDescriptorPool(device, pool.handle, nullptr);
                }
                mPools.clear();
                mDescriptorSets.Clear();
                mSetPools.clear();
                mFreeIDs.clear();
                clearDescriptors.Clear();
            }

          private:
            struct Pool {
                VkDescriptorPool handle;
                uint32_t setCount;
            };

            static void AddPool() {
                VkDescriptorPoolSize poolSize{};
                poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSize.descriptorCount = setsPerPool * mapCount;

                VkDescriptorPoolCreateInfo info{};
                info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                info.maxSets       = setsPerPool;
                info.poolSizeCount = 1;
                info.pPoolSizes    = &poolSize;

                ADH_THROW(vkCreateDescriptorPool(Context::Get()->GetDevice(), &info, nullptr, &mPools.emplace_back().handle) == VK_SUCCESS,
                          "Failed to create descriptor pool!");
            }

          private:
            inline static std::vector<Pool> mPools;
            inline static VkDescriptorSetLayout setLayout;
            inline static Array<VkDescriptorSet> mDescriptorSets;
            inline static std::vector<VkDescriptorPool> mSetPools;
            inline static std::vector<uint32_t> mFreeIDs;

            inline static Array<uint32_t> clearDescriptors;
        };
    } // namespace vk
} // namespace adh

namespace adh {
    namespace vk {
        class DescriptorSet {
          public:
            DescriptorSet() noexcept;

            DescriptorSet(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, std::uint32_t swapchainImageViews);

            DescriptorSet(const DescriptorSet&) = delete;

            DescriptorSet& operator=(const DescriptorSet&) = delete;

            DescriptorSet(DescriptorSet&& rhs) noexcept;

            DescriptorSet& operator=(DescriptorSet&& rhs) noexcept;

            ~DescriptorSet();

            void Initialize(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, std::uint32_t swapchainImageViews);

            void Create(const Array<VkDescriptorSetLayout>& descriptorSetLayout);

            void AddPool(VkDescriptorType type, std::uint32_t count);

            void Update(
                VkBuffer buffer,
                VkDeviceSize offset,
                VkDeviceSize range,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Update(
                VkDescriptorBufferInfo bufferInfo,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Update(
                const VkDescriptorBufferInfo* bufferInfo,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Update(
                const UniformBuffer& uniformBuffer,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Update(
                VkSampler sampler,
                VkImageView imageView,
                VkImageLayout imageLayout,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Update( // TODO: array
                VkDescriptorImageInfo imageInfo,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Update(
                const VkDescriptorImageInfo* imageInfo,
                std::uint32_t dstSetIndex,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t arrayCount,
                VkDescriptorType type);

            void Bind(VkCommandBuffer commandBuffer, std::uint32_t imageIndex) ADH_NOEXCEPT;

            void Destroy() noexcept;

            VkDescriptorSet& GetSet(std::uint32_t setIndex, std::uint32_t imageIndex) noexcept;

            const VkDescriptorSet& GetSet(std::uint32_t setIndex, std::uint32_t imageIndex) const noexcept;

            VkDescriptorPool GetPool() noexcept;

            VkDescriptorPool GetPool() const noexcept;

          private:
            void CreatePool(std::uint32_t maxSize) ADH_NOEXCEPT;

            void AllocateDescriptors(const Array<VkDescriptorSetLayout>& descriptorSetLayout);

            void MoveConstruct(DescriptorSet&& rhs) noexcept;

            void Clear() noexcept;

          public:
            VkDescriptorPool m_Pool;
            VkPipelineLayout m_PipelineLayout;
            VkPipelineBindPoint m_BindPoint;
            std::uint32_t m_SwapChainImageViews;
            Array<VkDescriptorPoolSize> m_PoolSizes;
            Array<VkDescriptorSet> m_DescriptorSets;
        };
    } // namespace vk
} // namespace adh
