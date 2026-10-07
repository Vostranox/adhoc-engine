#include "Instance.hpp"
#include "Initializers.hpp"

#include <Std/Array.hpp>
#include <Utility.hpp>

namespace adh {
    namespace vk {
        Instance::Instance() noexcept : m_Instance{ VK_NULL_HANDLE } {
        }

        Instance::Instance(const char* name) {
            Create(name);
        }

        Instance::Instance(Instance&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Instance& Instance::operator=(Instance&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        Instance::~Instance() {
            Clear();
        }

        void Instance::Create(const char*) {
            auto applicationInfo{ initializers::ApplicationInfo("AdHoc", VK_API_VERSION_1_2) };
            Array<const char*> validationLayers;
#if defined(ADH_DEBUG)
            validationLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
#endif
            Array<const char*> instanceExtentions{ VK_KHR_SURFACE_EXTENSION_NAME };
#if defined(ADH_WINDOWS)
            instanceExtentions.EmplaceBack(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(ADH_APPLE)
            instanceExtentions.EmplaceBack(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
            instanceExtentions.EmplaceBack("VK_KHR_portability_enumeration");
#elif defined(ADH_LINUX)
            instanceExtentions.EmplaceBack(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
            auto instanceCreateInfo{ initializers::InstanceCreateInfo(applicationInfo, instanceExtentions, validationLayers) };
            ADH_THROW(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance) == VK_SUCCESS,
                      "Failed to create VkInstance!");
        }

        void Instance::Destroy() noexcept {
            Clear();
        }

        Instance::operator VkInstance() noexcept {
            return m_Instance;
        }

        Instance::operator VkInstance() const noexcept {
            return m_Instance;
        }

        void Instance::MoveConstruct(Instance&& rhs) noexcept {
            m_Instance     = rhs.m_Instance;
            rhs.m_Instance = VK_NULL_HANDLE;
        }

        void Instance::Clear() noexcept {
            if (m_Instance != VK_NULL_HANDLE) {
                vkDestroyInstance(m_Instance, nullptr);
                m_Instance = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
