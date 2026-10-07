#include "Context.hpp"
#include "Allocator.hpp"
#include <Utf8.hpp>

#if defined(ADH_WINDOWS)
#    include <Windows.h>
#elif defined(ADH_APPLE)
#    include <mach-o/dyld.h>
#endif

#include <cstdint>
#include <filesystem>
#include <system_error>
#include <vector>

namespace adh {
    namespace vk {
        static std::filesystem::path ExecutableDirectory(const char* path) {
            std::filesystem::path executable;
#if defined(ADH_WINDOWS)
            wchar_t buffer[MAX_PATH]{};
            if (GetModuleFileNameW(nullptr, buffer, MAX_PATH) != 0) {
                executable = buffer;
            }
#elif defined(ADH_APPLE)
            std::uint32_t length{};
            _NSGetExecutablePath(nullptr, &length);
            std::vector<char> buffer(length);
            if (_NSGetExecutablePath(buffer.data(), &length) == 0) {
                executable = PathFromUtf8(buffer.data());
            }
#elif defined(ADH_LINUX)
            std::error_code error;
            executable = std::filesystem::read_symlink("/proc/self/exe", error);
#endif
            if (executable.empty() && path) {
                executable = PathFromUtf8(path);
            }
            return std::filesystem::weakly_canonical(std::filesystem::absolute(executable)).parent_path();
        }

        Context::Context(const Window& window, const char* name, const char* path) {
            Create(window, name, path);
        }

        Context::~Context() {
            Clear();
        }

        void Context::Create(const Window& window, const char* name, const char* path) {
            m_Path = ToUtf8((ExecutableDirectory(path) / PathFromUtf8(DATA_DIRECTORY)).lexically_normal());
            if (!m_Path.ends_with('/')) {
                m_Path += '/';
            }
            m_Contexts.EmplaceBack(this);
            m_Instance.Create(name);
            m_PhysicalDevice.Create(m_Instance);
            m_Surface.Create(m_Instance, window);
            m_DeviceQueues.Create(m_PhysicalDevice, m_Surface);
            m_Device.Create(m_PhysicalDevice, &m_DeviceQueues);
        }

        void Context::Destroy() noexcept {
            Clear();
        }

        void Context::Clear() noexcept {
            Allocator::Destroy();
            m_Surface.Destroy();
            m_Device.Destroy();
            m_Instance.Destroy();
        }

        Context* Context::Get() noexcept {
            return m_Contexts[0];
        }

        VkInstance Context::GetInstance() noexcept {
            return m_Instance;
        }

        VkInstance Context::GetInstance() const noexcept {
            return m_Instance;
        }

        VkPhysicalDevice Context::GetPhysicalDevice() noexcept {
            return m_PhysicalDevice;
        }

        VkPhysicalDevice Context::GetPhysicalDevice() const noexcept {
            return m_PhysicalDevice;
        }

        VkSurfaceKHR Context::GetSurface() noexcept {
            return m_Surface;
        }

        VkSurfaceKHR Context::GetSurface() const noexcept {
            return m_Surface;
        }

        DeviceQueueData Context::GetQueue(DeviceQueues::Family index) noexcept {
            return m_DeviceQueues[index];
        }

        const DeviceQueueData Context::GetQueue(DeviceQueues::Family index) const noexcept {
            return m_DeviceQueues[index];
        }

        VkDevice Context::GetDevice() noexcept {
            return m_Device;
        }

        VkDevice Context::GetDevice() const noexcept {
            return m_Device;
        }

        const std::string Context::GetDataDirectory() const noexcept {
            return m_Path;
        }
    } // namespace vk
} // namespace adh
