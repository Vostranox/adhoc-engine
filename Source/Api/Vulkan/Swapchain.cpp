#include "Swapchain.hpp"
#include "Context.hpp"
#include "Initializers.hpp"
#include "Tools.hpp"

namespace adh {
    namespace vk {
        Swapchain::Swapchain() noexcept : m_Swapchain{ VK_NULL_HANDLE },
                                          m_Format{},
                                          m_PresentMode{},
                                          m_Extent{} {
        }

        Swapchain::Swapchain(std::uint32_t imageBuffersCount, VkFormat format, VkPresentModeKHR presentMode) {
            Create(imageBuffersCount, format, presentMode);
        }

        Swapchain::Swapchain(Swapchain&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Swapchain& Swapchain::operator=(Swapchain&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        Swapchain::~Swapchain() {
            Clear();
        }

        void Swapchain::Create(std::uint32_t imageBuffersCount, VkFormat format, VkPresentModeKHR presentMode) {
            m_ImageBuffersCount = imageBuffersCount;
            auto context{ Context::Get() };
            FindFormat(format, context->GetPhysicalDevice(), context->GetSurface());
            FindPresentMode(presentMode, context->GetPhysicalDevice(), context->GetSurface());
            CreateSwapchain(VK_NULL_HANDLE);
            CreateImageViews();

            isValid = true;
        }

        void Swapchain::Destroy() noexcept {
            Clear();
        }

        VkSwapchainKHR Swapchain::Get() noexcept {
            return m_Swapchain;
        }

        VkSwapchainKHR Swapchain::Get() const noexcept {
            return m_Swapchain;
        }

        Array<VkImageView>& Swapchain::GetImageView() noexcept {
            return m_ImageViews;
        }

        const Array<VkImageView>& Swapchain::GetImageView() const noexcept {
            return m_ImageViews;
        }

        VkFormat Swapchain::GetFormat() const noexcept {
            return m_Format.format;
        }

        VkExtent2D Swapchain::GetExtent() const noexcept {
            return m_Extent;
        }

        Image& Swapchain::GetColorBuffer() noexcept {
            return m_ColorBuffer;
        }

        std::uint32_t Swapchain::GetImageViewCount() const noexcept {
            return static_cast<std::uint32_t>(m_ImageViews.GetSize());
        }

        Swapchain::operator VkSwapchainKHR() noexcept {
            return m_Swapchain;
        }

        Swapchain::operator VkSwapchainKHR() const noexcept {
            return m_Swapchain;
        }

        void Swapchain::FindFormat(VkFormat format, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
            auto surfaceFormats{ tools::GetPhysicalDeviceSurfaceFormats(physicalDevice, surface) };
            bool found{};
            for (std::size_t i{}; i != surfaceFormats.GetSize(); ++i) {
                if (surfaceFormats[i].format == format) {
                    m_Format = surfaceFormats[i];
                    found    = true;
                    break;
                }
            }
            ADH_THROW(found, "Failed to find desired format!");
        }

        void Swapchain::FindPresentMode(VkPresentModeKHR presentMode, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
            auto presentModes{ tools::GetPhysicalDeviceSurfacePresentModes(physicalDevice, surface) };
            bool found{};
            for (std::size_t i{}; i != presentModes.GetSize(); ++i) {
                if (presentModes[i] == presentMode) {
                    m_PresentMode = presentModes[i];
                    found         = true;
                    break;
                }
            }
            if (!found) {
                m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
            }
        }

        void Swapchain::CreateSwapchain(VkSwapchainKHR oldSwapchain) {
            auto context{ Context::Get() };
            auto surfaceCapabilities{ tools::GetSurfaceCapabilities(context->GetPhysicalDevice(), context->GetSurface()) };
            std::uint32_t queueIndices[2]{ context->GetQueue(DeviceQueues::Family::eGraphics).index.value(),
                                           context->GetQueue(DeviceQueues::Family::ePrensent).index.value() };
            auto swapchainCreateInfo    = initializers::SwapchainCreateInfo(queueIndices, oldSwapchain);
            swapchainCreateInfo.surface = context->GetSurface();
            if (m_ImageBuffersCount < surfaceCapabilities.minImageCount) {
                swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount;
            } else {
                swapchainCreateInfo.minImageCount = m_ImageBuffersCount;
            }
            swapchainCreateInfo.imageFormat     = m_Format.format;
            swapchainCreateInfo.imageColorSpace = m_Format.colorSpace;
            swapchainCreateInfo.imageExtent = m_Extent = surfaceCapabilities.currentExtent;
            swapchainCreateInfo.presentMode            = m_PresentMode;
            ADH_THROW(vkCreateSwapchainKHR(context->GetDevice(), &swapchainCreateInfo, nullptr, &m_Swapchain) == VK_SUCCESS,
                      "Failed to create swapchain!");
        }

        void Swapchain::CreateImageViews() {
            m_Images = tools::GetSwapchainImages(Context::Get()->GetDevice(), m_Swapchain);
            m_ImageViews.Resize(m_Images.GetSize());

            for (std::size_t i{}; i != m_ImageViews.GetSize(); ++i) {
                auto viewInfo{ initializers::ImageViewCreateInfo(m_Images[i], VK_IMAGE_VIEW_TYPE_2D, m_Format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1u, 1u) };
                ADH_THROW(vkCreateImageView(Context::Get()->GetDevice(), &viewInfo, nullptr, &m_ImageViews[i]) == VK_SUCCESS,
                          "Failed to create image view!");
            }
        }

        void Swapchain::MoveConstruct(Swapchain&& rhs) noexcept {
            m_Swapchain         = rhs.m_Swapchain;
            m_Format            = rhs.m_Format;
            m_PresentMode       = rhs.m_PresentMode;
            m_Extent            = rhs.m_Extent;
            m_Images            = Move(rhs.m_Images);
            m_ImageViews        = Move(rhs.m_ImageViews);
            m_ImageBuffersCount = rhs.m_ImageBuffersCount;

            rhs.m_Swapchain = VK_NULL_HANDLE;
        }

        void Swapchain::Clear() {
            auto device{ Context::Get()->GetDevice() };
            vkDeviceWaitIdle(device);

            for (std::uint32_t i{}; i != m_ImageViews.GetSize(); ++i) {
                vkDestroyImageView(device, m_ImageViews[i], nullptr);
                m_ImageViews[i] = VK_NULL_HANDLE;
            }
            m_ImageViews.Clear();

            if (m_Swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
                m_Swapchain = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
