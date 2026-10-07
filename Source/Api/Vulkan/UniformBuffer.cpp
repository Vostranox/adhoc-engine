#include "UniformBuffer.hpp"
#include "Context.hpp"
#include "Tools.hpp"

#include <algorithm>
#include <cstring>

namespace adh {
    namespace vk {
        UniformBuffer::UniformBuffer() noexcept : m_Descriptor{},
                                                  m_Stride{},
                                                  m_Count{},
                                                  m_Data{},
                                                  m_MappedPtr{} {
        }

        UniformBuffer::UniformBuffer(const void* data, std::size_t size, std::uint32_t count, VkBufferUsageFlagBits bufferUsage) {
            Create(data, size, count, bufferUsage);
        }

        UniformBuffer::UniformBuffer(UniformBuffer&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        UniformBuffer& UniformBuffer::operator=(UniformBuffer&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        UniformBuffer::~UniformBuffer() {
            Clear();
        }

        void UniformBuffer::Create(const void* data, std::size_t size, std::uint32_t count, VkBufferUsageFlagBits bufferUsage) {
            auto physicalDevice{ Context::Get()->GetPhysicalDevice() };
            auto memoryProperty{ tools::IsUniformMemoryAccess(physicalDevice) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0 };

            const auto limits{ tools::GetPhysicalDeviceProperties(physicalDevice).limits };
            VkDeviceSize alignment{ 1u };
            if (bufferUsage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
                alignment = std::max(alignment, limits.minUniformBufferOffsetAlignment);
            }
            if (bufferUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
                alignment = std::max(alignment, limits.minStorageBufferOffsetAlignment);
            }
            m_Stride = (size + alignment - 1u) & ~(alignment - 1u);
            m_Count  = count;

            m_Buffer.Create(m_Stride, count, bufferUsage, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | memoryProperty));
            m_Data = data;
            m_Buffer.Map(nullptr, m_MappedPtr);

            m_Descriptor.buffer = m_Buffer;
            m_Descriptor.offset = 0u;
            m_Descriptor.range  = static_cast<VkDeviceSize>(size);

            if (m_Data) {
                for (std::uint32_t i{}; i != count; ++i) {
                    Update(i);
                }
            }
        }

        void UniformBuffer::Destroy() noexcept {
            Clear();
        }

        void UniformBuffer::Update(std::uint32_t imageIndex) ADH_NOEXCEPT {
            Update(m_Data, imageIndex);
        }

        void UniformBuffer::Update(const void* data, std::uint32_t imageIndex) ADH_NOEXCEPT {
            ADH_THROW(imageIndex < m_Count, "Uniform buffer index out of range!");
            const std::size_t offset{ m_Stride * imageIndex };
            void* const ptr{ static_cast<char*>(m_MappedPtr) + offset };
            std::memcpy(ptr, data, static_cast<std::size_t>(m_Descriptor.range));
        }

        VkDescriptorBufferInfo UniformBuffer::GetDescriptor(std::uint32_t imageIndex) const noexcept {
            auto descriptor{ m_Descriptor };
            descriptor.offset = m_Stride * imageIndex;
            return descriptor;
        }

        std::uint32_t UniformBuffer::GetCount() const noexcept {
            return m_Count;
        }

        std::uint64_t UniformBuffer::GetSize() const noexcept {
            return static_cast<std::uint64_t>(m_Descriptor.range);
        }

        void* UniformBuffer::GetMappedPtr() noexcept {
            return m_MappedPtr;
        }

        const void* UniformBuffer::GetMappedPtr() const noexcept {
            return m_MappedPtr;
        }

        UniformBuffer::operator VkBuffer() noexcept {
            return m_Buffer;
        }

        UniformBuffer::operator VkBuffer() const noexcept {
            return m_Buffer;
        }

        void UniformBuffer::MoveConstruct(UniformBuffer&& rhs) noexcept {
            m_Buffer     = Move(rhs.m_Buffer);
            m_Descriptor = rhs.m_Descriptor;
            m_Stride     = rhs.m_Stride;
            m_Count      = rhs.m_Count;
            m_Data       = rhs.m_Data;
            m_MappedPtr  = rhs.m_MappedPtr;

            rhs.m_Descriptor = {};
            rhs.m_Stride     = 0u;
            rhs.m_Count      = 0u;
            rhs.m_Data       = nullptr;
            rhs.m_MappedPtr  = nullptr;
        }

        void UniformBuffer::Clear() noexcept {
            m_Buffer.Destroy();
            m_Descriptor = {};
            m_Stride     = 0u;
            m_Count      = 0u;
            m_Data       = nullptr;
            m_MappedPtr  = nullptr;
        }
    } // namespace vk
} // namespace adh
