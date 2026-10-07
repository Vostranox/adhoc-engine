#pragma once
#include <vulkan/vulkan.h>

namespace adh {
    namespace vk {
        class Instance {
          public:
          public:
            Instance() noexcept;

            Instance(const char* name);

            Instance(const Instance&) = delete;

            Instance& operator=(const Instance&) = delete;

            Instance(Instance&& rhs) noexcept;

            Instance& operator=(Instance&& rhs) noexcept;

            ~Instance();

            void Create(const char* name);

            void Destroy() noexcept;

            operator VkInstance() noexcept;

            operator VkInstance() const noexcept;

          private:
            void MoveConstruct(Instance&& rhs) noexcept;

            void Clear() noexcept;

          private:
            VkInstance m_Instance;
        };
    } // namespace vk
} // namespace adh
