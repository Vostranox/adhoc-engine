#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#if defined(ADH_WINDOWS)
#    include <Windows.h>
#elif defined(ADH_LINUX)
#    include <xcb/xcb.h>
#endif

#include <Std/Array.hpp>
#include <cstdint>

namespace adh {
    namespace vk {
        namespace initializers {
            inline auto ApplicationInfo(const char* applicationName, std::uint32_t apiVersion) {
                VkApplicationInfo info{
                    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .pNext              = nullptr,
                    .pApplicationName   = applicationName,
                    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                    .pEngineName        = "No Engine",
                    .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
                    .apiVersion         = apiVersion
                };
                return info;
            }

            inline auto InstanceCreateInfo(
                const VkApplicationInfo& applicationInfo,
                const Array<const char*>& instanceExtentions,
                const Array<const char*>& validationLayers) {
#if defined(ADH_APPLE)
                constexpr VkInstanceCreateFlags flags{ VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR };
#else
                constexpr VkInstanceCreateFlags flags{};
#endif
                VkInstanceCreateInfo info{
                    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .pNext                   = nullptr,
                    .flags                   = flags,
                    .pApplicationInfo        = &applicationInfo,
                    .enabledLayerCount       = static_cast<std::uint32_t>(validationLayers.GetSize()),
                    .ppEnabledLayerNames     = validationLayers.GetData(),
                    .enabledExtensionCount   = static_cast<std::uint32_t>(instanceExtentions.GetSize()),
                    .ppEnabledExtensionNames = instanceExtentions.GetData()
                };
                return info;
            }

#if defined(ADH_WINDOWS)
            inline auto Win32SurfaceCreateInfo(HWND hwnd, HINSTANCE hinstance) noexcept {
                VkWin32SurfaceCreateInfoKHR info{
                    .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                    .pNext     = nullptr,
                    .flags     = 0u,
                    .hinstance = hinstance,
                    .hwnd      = hwnd
                };
                return info;
            }

#elif defined(ADH_APPLE)
            inline auto MacOSSurfaceCreateInfo(void* view) noexcept {
                VkMacOSSurfaceCreateInfoMVK info{
                    .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
                    .pNext = nullptr,
                    .flags = 0u,
                    .pView = view
                };
                return info;
            }
#elif defined(ADH_LINUX)
            inline auto XcbSurfaceCreateInfo(xcb_connection_t* connection, xcb_window_t window) noexcept {
                VkXcbSurfaceCreateInfoKHR info = {};
                info.sType                     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
                info.connection                = connection;
                info.window                    = window;
                return info;
            }
#endif
            inline auto DeviceQueueCreateInfo(std::uint32_t queueFamilyIndex) noexcept {
                static constexpr float queuePriority[]{ 1.0f };
                VkDeviceQueueCreateInfo queueCreateInfo{
                    .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext            = nullptr,
                    .flags            = 0u,
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount       = 1u,
                    .pQueuePriorities = queuePriority
                };
                return queueCreateInfo;
            }

            inline auto DeviceCreateInfo(const Array<const char*>& deviceExtentions) noexcept {
                VkDeviceCreateInfo deviceCreateInfo{};
                deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                deviceCreateInfo.pNext                   = nullptr;
                deviceCreateInfo.flags                   = 0u;
                deviceCreateInfo.enabledLayerCount       = 0;
                deviceCreateInfo.ppEnabledLayerNames     = nullptr;
                deviceCreateInfo.enabledExtensionCount   = static_cast<std::uint32_t>(deviceExtentions.GetSize());
                deviceCreateInfo.ppEnabledExtensionNames = deviceExtentions.GetData();
                return deviceCreateInfo;
            }

            inline auto PhysicalDeviceBufferDeviceAddressFeatures() noexcept {
                VkPhysicalDeviceBufferDeviceAddressFeaturesEXT bufferDeviceAddressFeatures{
                    .sType                            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT,
                    .pNext                            = nullptr,
                    .bufferDeviceAddress              = VK_TRUE,
                    .bufferDeviceAddressCaptureReplay = VK_FALSE,
                    .bufferDeviceAddressMultiDevice   = VK_FALSE
                };
                return bufferDeviceAddressFeatures;
            }

            inline auto PhysicalDeviceRayTracingPipelineFeatures(VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& bufferDeviceAddressFeatures) noexcept {
                VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
                rayTracingPipelineFeatures.sType              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                rayTracingPipelineFeatures.pNext              = &bufferDeviceAddressFeatures;
                rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
                return rayTracingPipelineFeatures;
            }

            inline auto PhysicalDeviceAccelerationStructureFeatures(VkPhysicalDeviceRayTracingPipelineFeaturesKHR& rayTracingPipelineFeatures) noexcept {
                VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{
                    .sType                                                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
                    .pNext                                                 = &rayTracingPipelineFeatures,
                    .accelerationStructure                                 = VK_TRUE,
                    .accelerationStructureCaptureReplay                    = VK_TRUE,
                    .accelerationStructureIndirectBuild                    = VK_FALSE,
                    .accelerationStructureHostCommands                     = VK_FALSE,
                    .descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE
                };
                return accelerationStructureFeatures;
            }

            inline auto SwapchainCreateInfo(std::uint32_t* queueIndices, VkSwapchainKHR oldSwapchain) noexcept {
                VkSwapchainCreateInfoKHR info{};
                info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                info.imageArrayLayers = 1u;
                info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                info.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                info.clipped          = VK_TRUE;
                info.oldSwapchain     = oldSwapchain;
                if (queueIndices[0] != queueIndices[1]) {
                    info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                    info.queueFamilyIndexCount = 2u;
                    info.pQueueFamilyIndices   = queueIndices;
                } else {
                    info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                    info.queueFamilyIndexCount = 0u;
                    info.pQueueFamilyIndices   = nullptr;
                }
                return info;
            }

            inline auto SemaphoreCreateInfo() noexcept {
                VkSemaphoreCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                return info;
            }

            inline auto FenceCreateInfo(VkFenceCreateFlags flag) noexcept {
                VkFenceCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                info.flags = flag;
                return info;
            }

            inline auto ImageViewCreateInfo(
                VkImage image,
                VkImageViewType imageViewType,
                VkFormat format,
                VkImageAspectFlagBits imageAspectFlag,
                std::uint32_t levelCount,
                std::uint32_t layerCount) {
                VkImageViewCreateInfo info{};
                info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                info.image                           = image;
                info.viewType                        = imageViewType;
                info.format                          = format;
                info.subresourceRange.aspectMask     = imageAspectFlag;
                info.subresourceRange.baseMipLevel   = 0u;
                info.subresourceRange.levelCount     = levelCount;
                info.subresourceRange.baseArrayLayer = 0u;
                info.subresourceRange.layerCount     = layerCount;
                return info;
            }

            inline auto SamplerCreateInfo(
                VkFilter magFilter,
                VkFilter minFilter,
                VkSamplerMipmapMode mipmapMode,
                VkSamplerAddressMode mode,
                VkCompareOp compareOp,
                VkBool32 useMipMap,
                float maxAnisotropy,
                VkBool32 anisotropyEnable) noexcept {
                VkSamplerCreateInfo info{};
                info.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                info.magFilter    = magFilter;
                info.minFilter    = minFilter;
                info.mipmapMode   = mipmapMode;
                info.addressModeU = mode;
                info.addressModeV = mode;
                info.addressModeW = mode;
                info.mipLodBias   = 0.0f;
                if (anisotropyEnable) {
                    info.anisotropyEnable = VK_TRUE;
                    info.maxAnisotropy    = maxAnisotropy;
                } else {
                    info.anisotropyEnable = VK_FALSE;
                    info.maxAnisotropy    = 0.0f;
                }
                info.compareEnable = compareOp != VK_COMPARE_OP_NEVER;
                info.compareOp     = compareOp;
                info.minLod        = 0.0f;
                if (useMipMap) {
                    info.maxLod = VK_LOD_CLAMP_NONE;
                } else {
                    info.maxLod = 1.0f;
                }
                info.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
                info.unnormalizedCoordinates = VK_FALSE;

                return info;
            }

            inline auto BufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage) noexcept {
                VkBufferCreateInfo info{};
                info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                info.size        = size;
                info.usage       = usage;
                info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                return info;
            }

            inline auto ImageCreateInfo(
                VkExtent3D extent,
                VkFormat format,
                VkImageTiling imageTiling,
                VkImageType imageType,
                VkImageCreateFlagBits imageCreateFlags,
                std::uint32_t mipLevels,
                std::uint32_t arrayLayers,
                VkSampleCountFlagBits samples,
                VkImageUsageFlagBits imageUsageFlag,
                VkSharingMode sharingMode) noexcept {
                VkImageCreateInfo info{};
                info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                info.pNext                 = nullptr;
                info.flags                 = imageCreateFlags;
                info.imageType             = imageType;
                info.format                = format;
                info.extent                = extent;
                info.mipLevels             = mipLevels;
                info.arrayLayers           = arrayLayers;
                info.samples               = samples;
                info.tiling                = imageTiling;
                info.usage                 = imageUsageFlag;
                info.sharingMode           = sharingMode;
                info.queueFamilyIndexCount = 0u;
                info.pQueueFamilyIndices   = nullptr;
                info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
                return info;
            }

            inline auto MemoryAllocateInfo(VkDeviceSize size, std::uint32_t type) noexcept {
                VkMemoryAllocateInfo info{};
                info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                info.allocationSize  = size;
                info.memoryTypeIndex = type;
                return info;
            }

            inline auto CommandPoolCreateInfo(VkCommandPoolCreateFlagBits flag, std::uint32_t queueFamilyIndex) noexcept {
                VkCommandPoolCreateInfo info{};
                info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                info.flags            = flag;
                info.queueFamilyIndex = queueFamilyIndex;
                return info;
            }

            inline auto CommandBufferAllocateInfo(VkCommandPool commandPool, std::uint32_t commandBufferCount, VkCommandBufferLevel level) noexcept {
                VkCommandBufferAllocateInfo info{};
                info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                info.commandPool        = commandPool;
                info.level              = level;
                info.commandBufferCount = commandBufferCount;
                return info;
            }

            inline auto CommandBufferBeginInfo(VkCommandBufferUsageFlagBits flag = {}) noexcept {
                VkCommandBufferBeginInfo info{};
                info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                info.flags = flag;
                return info;
            }

            inline auto DescriptorPoolCreateInfo(std::uint32_t maxSets, std::uint32_t count, const VkDescriptorPoolSize* poolSizes) noexcept {
                VkDescriptorPoolCreateInfo info{};
                info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                info.maxSets       = maxSets;
                info.poolSizeCount = count;
                info.pPoolSizes    = poolSizes;
                return info;
            }

            inline auto DescriptorSetAllocateInfo(VkDescriptorPool pool, std::uint32_t count, const VkDescriptorSetLayout* setLayouts) noexcept {
                VkDescriptorSetAllocateInfo info{};
                info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                info.descriptorPool     = pool;
                info.descriptorSetCount = count;
                info.pSetLayouts        = setLayouts;
                return info;
            }

            inline auto DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout) noexcept {
                VkDescriptorImageInfo info{};
                info.sampler     = sampler;
                info.imageView   = imageView;
                info.imageLayout = imageLayout;
                return info;
            }

            inline auto DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) noexcept {
                VkDescriptorBufferInfo info{};
                info.buffer = buffer;
                info.offset = offset;
                info.range  = range;
                return info;
            }

            inline auto WriteDescriptorSet(
                VkDescriptorSet dstSet,
                std::uint32_t dstBinding,
                std::uint32_t arrayElement,
                std::uint32_t count,
                VkDescriptorType type,
                const VkDescriptorImageInfo* imageInfo,
                const VkDescriptorBufferInfo* bufferInfo,
                const VkBufferView* texelBufferView) noexcept

            {
                VkWriteDescriptorSet writeSet{};
                writeSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeSet.dstSet           = dstSet;
                writeSet.dstBinding       = dstBinding;
                writeSet.dstArrayElement  = arrayElement;
                writeSet.descriptorCount  = count;
                writeSet.descriptorType   = type;
                writeSet.pImageInfo       = imageInfo;
                writeSet.pBufferInfo      = bufferInfo;
                writeSet.pTexelBufferView = texelBufferView;
                return writeSet;
            }

            inline auto AttachmentDescription(
                VkFormat format,
                VkSampleCountFlagBits samples,
                VkAttachmentLoadOp loadOp,
                VkAttachmentStoreOp storeOp,
                VkAttachmentLoadOp stencilLoadOp,
                VkAttachmentStoreOp stencilStoreOp,
                VkImageLayout finalLayout,
                VkImageLayout initialLayout) noexcept {
                VkAttachmentDescription description{};
                description.format         = format;
                description.samples        = samples;
                description.loadOp         = loadOp;
                description.storeOp        = storeOp;
                description.stencilLoadOp  = stencilLoadOp;
                description.stencilStoreOp = stencilStoreOp;
                description.initialLayout  = initialLayout;
                description.finalLayout    = finalLayout;
                return description;
            }

            inline auto AttachmentReference(std::uint32_t attachmentIndex, VkImageLayout layout) noexcept {
                VkAttachmentReference reference{};
                reference.attachment = attachmentIndex;
                reference.layout     = layout;
                return reference;
            }

            inline auto SubpassDescription(
                VkPipelineBindPoint bindPoint,
                std::uint32_t colorAttachmentCount,
                const VkAttachmentReference* colorAttachments,
                const VkAttachmentReference* resolveAttachments,
                const VkAttachmentReference* depthStencilAttachment,
                std::uint32_t inputAttachmentCount,
                const VkAttachmentReference* inputAttachments) noexcept {
                VkSubpassDescription description{};
                description.pipelineBindPoint       = bindPoint;
                description.inputAttachmentCount    = inputAttachmentCount;
                description.pInputAttachments       = inputAttachments;
                description.colorAttachmentCount    = colorAttachmentCount;
                description.pColorAttachments       = colorAttachments;
                description.pResolveAttachments     = resolveAttachments;
                description.pDepthStencilAttachment = depthStencilAttachment;
                description.preserveAttachmentCount = 0u;
                description.pPreserveAttachments    = nullptr;
                return description;
            }

            inline auto SubpassDependency(
                std::uint32_t srcSubpass,
                std::uint32_t dstSubpass,
                VkPipelineStageFlags srcStageMask,
                VkPipelineStageFlags dstStageMask,
                VkAccessFlags srcAccessMask,
                VkAccessFlags dstAccessMask,
                VkDependencyFlags dependencyFlags) noexcept {
                VkSubpassDependency dependency{};
                dependency.srcSubpass      = srcSubpass;
                dependency.dstSubpass      = dstSubpass;
                dependency.srcStageMask    = srcStageMask;
                dependency.dstStageMask    = dstStageMask;
                dependency.srcAccessMask   = srcAccessMask;
                dependency.dstAccessMask   = dstAccessMask;
                dependency.dependencyFlags = dependencyFlags;
                return dependency;
            }

            inline auto RenderPassCreateInfo(
                std::uint32_t attachmentCount,
                const VkAttachmentDescription* attachments,
                std::uint32_t subpassCount,
                const VkSubpassDescription* subpasses,
                std::uint32_t dependencyCount,
                const VkSubpassDependency* dependencies) noexcept {
                VkRenderPassCreateInfo info{};
                info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                info.attachmentCount = attachmentCount;
                info.pAttachments    = attachments;
                info.subpassCount    = subpassCount;
                info.pSubpasses      = subpasses;
                info.dependencyCount = dependencyCount;
                info.pDependencies   = dependencies;
                return info;
            }

            inline auto RenderPassCreateInfo(
                const Array<VkAttachmentDescription>& attachments,
                const Array<VkSubpassDescription>& subpasses,
                const Array<VkSubpassDependency>& dependencies) noexcept {
                VkRenderPassCreateInfo info{};
                info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                info.attachmentCount = static_cast<std::uint32_t>(attachments.GetSize());
                info.pAttachments    = attachments.GetData();
                info.subpassCount    = static_cast<std::uint32_t>(subpasses.GetSize());
                info.pSubpasses      = subpasses.GetData();
                info.dependencyCount = static_cast<std::uint32_t>(dependencies.GetSize());
                info.pDependencies   = dependencies.GetData();
                return info;
            }

            inline auto Viewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) noexcept {
                VkViewport viewport{};
                viewport.x        = x;
                viewport.y        = y;
                viewport.width    = width;
                viewport.height   = height;
                viewport.minDepth = minDepth;
                viewport.maxDepth = maxDepth;
                return viewport;
            }

            inline auto Rect2D(VkExtent2D extent, VkOffset2D offset = { 0, 0 }) noexcept {
                VkRect2D rect{};
                rect.extent = extent;
                rect.offset = offset;
                return rect;
            }

            inline auto RenderPassBeginInfo(
                VkRenderPass renderPass,
                VkFramebuffer framebuffer,
                VkRect2D renderArea,
                std::uint32_t clearValueCount,
                const VkClearValue* clearValues) noexcept {
                VkRenderPassBeginInfo info{};
                info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                info.renderPass      = renderPass;
                info.framebuffer     = framebuffer;
                info.renderArea      = renderArea;
                info.clearValueCount = clearValueCount;
                info.pClearValues    = clearValues;
                return info;
            }

            inline auto PipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkBool32 restartEnable = VK_FALSE) noexcept {
                VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
                inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.topology               = topology;
                inputAssembly.primitiveRestartEnable = restartEnable;
                return inputAssembly;
            }

            inline auto PipelineViewportStateCreateInfo(
                std::uint32_t viewportCount = 1u,
                const VkViewport* viewports = nullptr,
                std::uint32_t scissorCount  = 1u,
                const VkRect2D* scissors    = nullptr) noexcept {
                VkPipelineViewportStateCreateInfo info{};
                info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                info.viewportCount = viewportCount;
                info.pViewports    = viewports;
                info.scissorCount  = scissorCount;
                info.pScissors     = scissors;
                return info;
            }

            inline auto PipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace) noexcept {
                VkPipelineRasterizationStateCreateInfo rasterizer{};
                rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable        = VK_FALSE;
                rasterizer.rasterizerDiscardEnable = VK_FALSE;
                rasterizer.polygonMode             = polygonMode;
                rasterizer.cullMode                = cullMode;
                rasterizer.frontFace               = frontFace;
                rasterizer.depthBiasEnable         = VK_TRUE;
                rasterizer.depthBiasConstantFactor = 0.0f;
                rasterizer.depthBiasClamp          = 0.0f;
                rasterizer.depthBiasSlopeFactor    = 0.0f;
                rasterizer.lineWidth               = 1.0f;
                return rasterizer;
            }

            inline auto PipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable, float minSampleShading) noexcept {
                VkPipelineMultisampleStateCreateInfo multisampling{};
                multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable   = sampleShadingEnable;
                multisampling.rasterizationSamples  = rasterizationSamples;
                multisampling.minSampleShading      = minSampleShading;
                multisampling.pSampleMask           = nullptr;
                multisampling.alphaToCoverageEnable = VK_FALSE;
                multisampling.alphaToOneEnable      = VK_FALSE;
                return multisampling;
            }

            inline auto PipelineDepthStencilStateCreateInfo(
                VkBool32 depthTestEnable,
                VkBool32 depthWriteEnable,
                VkCompareOp depthCompareOp,
                VkBool32 stencilTestEnable,
                const VkStencilOpState& stencil) noexcept {
                VkPipelineDepthStencilStateCreateInfo deapthStencil{};
                deapthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                deapthStencil.depthTestEnable       = depthTestEnable;
                deapthStencil.depthWriteEnable      = depthWriteEnable;
                deapthStencil.depthCompareOp        = depthCompareOp;
                deapthStencil.depthBoundsTestEnable = VK_FALSE;
                deapthStencil.stencilTestEnable     = stencilTestEnable;
                deapthStencil.front                 = stencil;
                deapthStencil.back                  = stencil;
                deapthStencil.minDepthBounds        = 0.0f;
                deapthStencil.maxDepthBounds        = 1.0f;
                return deapthStencil;
            }

            inline auto PipelineColorBlendAttachmentState(VkBool32 blendEnable) noexcept {
                VkPipelineColorBlendAttachmentState colorBlendAttachment{};
                if (blendEnable) {
                    colorBlendAttachment.blendEnable         = blendEnable;
                    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
                    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
                    colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                } else {
                    colorBlendAttachment.blendEnable    = blendEnable;
                    colorBlendAttachment.colorWriteMask = 0xf;
                }
                return colorBlendAttachment;
            }

            inline auto PipelineColorBlendStateCreateInfo(std::uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments) noexcept {
                VkPipelineColorBlendStateCreateInfo colorBlending{};
                colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.attachmentCount = attachmentCount;
                colorBlending.pAttachments    = pAttachments;
                return colorBlending;
            }

            inline auto PipelineDynamicStateCreateInfo(const VkDynamicState* dynamicStates, std::uint32_t dynamicStateCount) noexcept {
                VkPipelineDynamicStateCreateInfo dynamicState{};
                dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.dynamicStateCount = dynamicStateCount;
                dynamicState.pDynamicStates    = dynamicStates;
                return dynamicState;
            }

            inline auto GraphicsPipelineCreateInfo(
                std::uint32_t stageCount,
                const VkPipelineShaderStageCreateInfo* stages,
                const VkPipelineVertexInputStateCreateInfo* vertexInputState,
                const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState,
                const VkPipelineTessellationStateCreateInfo* tessellationState,
                const VkPipelineViewportStateCreateInfo* viewportState,
                const VkPipelineRasterizationStateCreateInfo* rasterizationState,
                const VkPipelineMultisampleStateCreateInfo* multisampleState,
                const VkPipelineDepthStencilStateCreateInfo* depthStencilState,
                const VkPipelineColorBlendStateCreateInfo* colorBlendState,
                const VkPipelineDynamicStateCreateInfo* dynamicState,
                VkPipelineLayout layout,
                VkRenderPass renderPass,
                std::uint32_t subpass          = 0u,
                VkPipeline basePipelineHandle  = VK_NULL_HANDLE,
                std::int32_t basePipelineIndex = 0u) noexcept {
                VkGraphicsPipelineCreateInfo pipelineInfo{};
                pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.stageCount          = stageCount;
                pipelineInfo.pStages             = stages;
                pipelineInfo.pVertexInputState   = vertexInputState;
                pipelineInfo.pInputAssemblyState = inputAssemblyState;
                pipelineInfo.pTessellationState  = tessellationState;
                pipelineInfo.pViewportState      = viewportState;
                pipelineInfo.pRasterizationState = rasterizationState;
                pipelineInfo.pMultisampleState   = multisampleState;
                pipelineInfo.pDepthStencilState  = depthStencilState;
                pipelineInfo.pColorBlendState    = colorBlendState;
                pipelineInfo.pDynamicState       = dynamicState;
                pipelineInfo.layout              = layout;
                pipelineInfo.renderPass          = renderPass;
                pipelineInfo.subpass             = subpass;
                pipelineInfo.basePipelineHandle  = basePipelineHandle;
                pipelineInfo.basePipelineIndex   = basePipelineIndex;
                return pipelineInfo;
            }

            inline auto ComputePipelineCreateInfo(VkPipelineLayout layout, VkPipelineShaderStageCreateInfo stage) noexcept {
                VkComputePipelineCreateInfo info{};
                info.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                info.layout = layout;
                info.stage  = stage;
                return info;
            }

            inline auto FramebufferCreateInfo(
                VkRenderPass renderPass,
                std::uint32_t attachmentCount,
                const VkImageView* attachments,
                VkExtent2D extent,
                std::uint32_t layers) noexcept {
                VkFramebufferCreateInfo info{};
                info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                info.renderPass      = renderPass;
                info.attachmentCount = attachmentCount;
                info.pAttachments    = attachments;
                info.width           = extent.width;
                info.height          = extent.height;
                info.layers          = layers;
                return info;
            }

            inline auto PresentInfo(
                std::uint32_t semaphoreCount,
                const VkSemaphore* semaphores,
                std::uint32_t swapchainCount,
                const VkSwapchainKHR* swapchains,
                const std::uint32_t* nextImage) noexcept {
                VkPresentInfoKHR info{};
                info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                info.waitSemaphoreCount = semaphoreCount;
                info.pWaitSemaphores    = semaphores;
                info.swapchainCount     = swapchainCount;
                info.pSwapchains        = swapchains;
                info.pImageIndices      = nextImage;
                return info;
            }

            inline auto BufferCopy(VkDeviceSize size, VkDeviceSize srcOffset = 0u, VkDeviceSize dstOffset = 0u) noexcept {
                VkBufferCopy bufferCopy{};
                bufferCopy.srcOffset = srcOffset;
                bufferCopy.dstOffset = dstOffset;
                bufferCopy.size      = size;
                return bufferCopy;
            }

            inline auto SubmitInfo(
                std::uint32_t commandBufferCount,
                const VkCommandBuffer* commandBuffers,
                std::uint32_t waitSemaphoreCount             = {},
                const VkSemaphore* waitSemaphores            = {},
                const VkPipelineStageFlags* waitDstStageMask = {},
                std::uint32_t signalSemaphoreCount           = {},
                const VkSemaphore* signalSemaphores          = {}) noexcept {
                VkSubmitInfo info{};
                info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                info.commandBufferCount   = commandBufferCount;
                info.pCommandBuffers      = commandBuffers;
                info.waitSemaphoreCount   = waitSemaphoreCount;
                info.pWaitSemaphores      = waitSemaphores;
                info.pWaitDstStageMask    = waitDstStageMask;
                info.signalSemaphoreCount = signalSemaphoreCount;
                info.pSignalSemaphores    = signalSemaphores;
                return info;
            }

            inline auto MappedMemoryRange(VkDeviceSize size, VkDeviceSize offset, VkDeviceMemory deviceMemory) noexcept {
                VkMappedMemoryRange mappedRange{};
                mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                mappedRange.memory = deviceMemory;
                mappedRange.offset = offset;
                mappedRange.size   = size;
                return mappedRange;
            }

            inline auto SpecializationMapEntry(std::size_t size) noexcept {
                VkSpecializationMapEntry specialization{};
                specialization.constantID = 0u;
                specialization.offset     = 0u;
                specialization.size       = size;
                return specialization;
            }

            inline auto SpecializationInfo(const VkSpecializationMapEntry* specializationEntry, std::size_t size, const void* data) noexcept {
                VkSpecializationInfo info{};
                info.mapEntryCount = 1u;
                info.pMapEntries   = specializationEntry;
                info.dataSize      = size;
                info.pData         = data;
                return info;
            }

            inline auto ShaderModuleCreateInfo(std::size_t size, const char* file) noexcept {
                VkShaderModuleCreateInfo info{};
                info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                info.pNext    = nullptr;
                info.flags    = 0;
                info.codeSize = size;
                info.pCode    = reinterpret_cast<const std::uint32_t*>(file);
                return info;
            }

            inline auto VertexInputBindingDescription(
                std::uint32_t binding,
                std::uint32_t stride,
                VkVertexInputRate inputRate) noexcept {
                VkVertexInputBindingDescription description{};
                description.binding   = binding;
                description.stride    = stride;
                description.inputRate = inputRate;
                return description;
            }

            inline auto VertexInputAttributeDescription(
                std::uint32_t location,
                std::uint32_t binding,
                VkFormat format,
                std::uint32_t offset) noexcept {
                VkVertexInputAttributeDescription description{};
                description.location = location;
                description.binding  = binding;
                description.format   = format;
                description.offset   = offset;
                return description;
            }

            inline auto PipelineVertexInputStateCreateInfo(
                VkVertexInputBindingDescription* bindingDescription,
                std::uint32_t bindingDestriptionCount,
                VkVertexInputAttributeDescription* atttributeDescription,
                std::uint32_t atttributeDescriptionCount) noexcept {
                VkPipelineVertexInputStateCreateInfo info{};
                info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                info.vertexBindingDescriptionCount   = bindingDestriptionCount;
                info.pVertexBindingDescriptions      = bindingDescription;
                info.vertexAttributeDescriptionCount = atttributeDescriptionCount;
                info.pVertexAttributeDescriptions    = atttributeDescription;
                return info;
            }

            inline auto PipelineVertexInputStateCreateInfo(
                const Array<VkVertexInputBindingDescription>& bindingDescriptions,
                const Array<VkVertexInputAttributeDescription>& atttributeDescriptions) noexcept {
                VkPipelineVertexInputStateCreateInfo info{};
                info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                info.vertexBindingDescriptionCount   = static_cast<std::uint32_t>(bindingDescriptions.GetSize());
                info.pVertexBindingDescriptions      = bindingDescriptions.GetData();
                info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(atttributeDescriptions.GetSize());
                info.pVertexAttributeDescriptions    = atttributeDescriptions.GetData();
                return info;
            }

            inline auto DescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutBinding* bindings, std::uint32_t bindingCount) noexcept {
                VkDescriptorSetLayoutCreateInfo info{};
                info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.bindingCount = bindingCount;
                info.pBindings    = bindings;
                return info;
            }

            inline auto DescriptorSetLayoutBinding(
                std::uint32_t binding,
                VkDescriptorType type,
                std::uint32_t count,
                VkShaderStageFlags stageFlag,
                const VkSampler* immutableSampler = nullptr) noexcept {
                VkDescriptorSetLayoutBinding layoutBinding{};
                layoutBinding.binding            = binding;
                layoutBinding.descriptorType     = type;
                layoutBinding.descriptorCount    = count;
                layoutBinding.stageFlags         = stageFlag;
                layoutBinding.pImmutableSamplers = immutableSampler;
                return layoutBinding;
            }

            inline auto PushConstantRange(VkShaderStageFlags stageFlag, std::uint32_t offset, std::uint32_t size) noexcept {
                VkPushConstantRange pushConstantRange{};
                pushConstantRange.stageFlags = stageFlag;
                pushConstantRange.offset     = offset;
                pushConstantRange.size       = size;
                return pushConstantRange;
            }

            inline auto PipelineLayoutCreateInfo(
                std::uint32_t setLayoutCount,
                const VkDescriptorSetLayout* setLayouts,
                std::uint32_t pushConstantRangeCount,
                const VkPushConstantRange* pushConstantRanges) noexcept {
                VkPipelineLayoutCreateInfo info{};
                info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                info.setLayoutCount         = setLayoutCount;
                info.pSetLayouts            = setLayouts;
                info.pushConstantRangeCount = pushConstantRangeCount;
                info.pPushConstantRanges    = pushConstantRanges;
                return info;
            }

            inline auto PipelineLayoutCreateInfo(
                const Array<VkDescriptorSetLayout>& setLayouts,
                const Array<VkPushConstantRange>& pushConstantRanges) noexcept {
                VkPipelineLayoutCreateInfo info{};
                info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                info.setLayoutCount         = static_cast<std::uint32_t>(setLayouts.GetSize());
                info.pSetLayouts            = setLayouts.GetData();
                info.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.GetSize());
                info.pPushConstantRanges    = pushConstantRanges.GetData();
                return info;
            }

            inline auto PipelineShaderStageCreateInfo(
                VkShaderStageFlagBits stage,
                VkShaderModule shaderModule,
                const VkSpecializationInfo* specializationInfo) noexcept {
                VkPipelineShaderStageCreateInfo info{};
                info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                info.stage               = stage;
                info.module              = shaderModule;
                info.pName               = "main";
                info.pSpecializationInfo = specializationInfo;
                return info;
            }

            inline auto MemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) noexcept {
                VkMemoryBarrier barrier{};
                barrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                barrier.srcAccessMask = srcAccessMask;
                barrier.dstAccessMask = dstAccessMask;
                return barrier;
            }

            inline auto BufferMemoryBarrier(
                VkAccessFlags srcAccessMask,
                VkAccessFlags dstAccessMask,
                uint32_t srcQueueFamilyIndex,
                uint32_t dstQueueFamilyIndex,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkDeviceSize size) noexcept {
                VkBufferMemoryBarrier barrier{};
                barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                barrier.srcAccessMask       = srcAccessMask;
                barrier.dstAccessMask       = dstAccessMask;
                barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
                barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
                barrier.buffer              = buffer;
                barrier.offset              = offset;
                barrier.size                = size;
                return barrier;
            }

            inline auto ImageMemoryBarrier(
                VkAccessFlags srcAccessMask,
                VkAccessFlags dstAccessMask,
                VkImageLayout oldLayout,
                VkImageLayout newLayout,
                std::uint32_t srcQueueFamilyIndex,
                std::uint32_t dstQueueFamilyIndex,
                VkImage image,
                VkImageAspectFlagBits aspectFlag,
                std::uint32_t baseMipLevel,
                std::uint32_t levelCount,
                std::uint32_t layerCount) noexcept {
                VkImageMemoryBarrier barrier{};
                barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.srcAccessMask                   = srcAccessMask;
                barrier.dstAccessMask                   = dstAccessMask;
                barrier.oldLayout                       = oldLayout;
                barrier.newLayout                       = newLayout;
                barrier.srcQueueFamilyIndex             = srcQueueFamilyIndex;
                barrier.dstQueueFamilyIndex             = dstQueueFamilyIndex;
                barrier.image                           = image;
                barrier.subresourceRange.aspectMask     = aspectFlag;
                barrier.subresourceRange.baseMipLevel   = baseMipLevel;
                barrier.subresourceRange.levelCount     = levelCount;
                barrier.subresourceRange.baseArrayLayer = 0u;
                barrier.subresourceRange.layerCount     = layerCount;
                return barrier;
            }

            inline auto BufferImageCopy(
                VkImageAspectFlagBits aspectFlag,
                VkExtent3D extent,
                std::uint32_t mipLevel,
                std::uint32_t layerCount) noexcept {
                VkBufferImageCopy copy{};
                copy.bufferOffset                    = 0u;
                copy.bufferRowLength                 = 0u;
                copy.bufferImageHeight               = 0u;
                copy.imageSubresource.aspectMask     = aspectFlag;
                copy.imageSubresource.mipLevel       = mipLevel;
                copy.imageSubresource.baseArrayLayer = 0u;
                copy.imageSubresource.layerCount     = layerCount;
                copy.imageOffset                     = { 0u, 0u, 0u };
                copy.imageExtent                     = extent;
                return copy;
            }

        } // namespace initializers
    }     // namespace vk
} // namespace adh
