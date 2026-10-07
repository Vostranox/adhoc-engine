#pragma once
#include "Device.hpp"
#include "DeviceQueues.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include <Std/Array.hpp>

#include <string>

namespace adh {
    namespace vk {
        class Context {
          public:
            Context() noexcept = default;

            Context(const Window& window, const char* name, const char* path);

            ~Context();

            void Create(const Window& window, const char* name, const char* path);

            void Destroy() noexcept;

            VkInstance GetInstance() noexcept;

            VkInstance GetInstance() const noexcept;

            VkPhysicalDevice GetPhysicalDevice() noexcept;

            VkPhysicalDevice GetPhysicalDevice() const noexcept;

            VkSurfaceKHR GetSurface() noexcept;

            VkSurfaceKHR GetSurface() const noexcept;

            DeviceQueueData GetQueue(DeviceQueues::Family index) noexcept;

            const DeviceQueueData GetQueue(DeviceQueues::Family index) const noexcept;

            VkDevice GetDevice() noexcept;

            VkDevice GetDevice() const noexcept;

            const std::string GetDataDirectory() const noexcept;

          private:
            void Clear() noexcept;

          public:
            static Context* Get() noexcept;

          private:
            Instance m_Instance;
            PhysicalDevice m_PhysicalDevice;
            Surface m_Surface;
            DeviceQueues m_DeviceQueues;
            Device m_Device;
            std::string m_Path;

          private:
            inline static Array<Context*> m_Contexts;
        };
    } // namespace vk
} // namespace adh
