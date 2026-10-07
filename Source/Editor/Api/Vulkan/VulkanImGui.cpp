#include "VulkanImGui.hpp"
#include "vulkan/vulkan_core.h"
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <Vulkan/Context.hpp>
#include <Vulkan/Memory.hpp>
#include <Vulkan/Shader.hpp>
#include <Vulkan/UniformBuffer.hpp>
#include <Vulkan/VertexLayout.hpp>

namespace adh {
    namespace vk {
        void VulkanImGui::Create(const RenderPass& renderPass, VkQueue, std::uint32_t imageViewCount) {
            m_VertexBuffer.Resize(imageViewCount);
            m_IndexBuffer.Resize(imageViewCount);
            m_DescriptorSet.Reserve(maxTextures);
            m_ImageViewCount = imageViewCount;

            ImGuiIO& io{ ImGui::GetIO() };

            unsigned char* fontData;
            int texWidth;
            int texHeight;
            io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
            auto uploadSize{ texWidth * texHeight * 4u };

            // Sampler
            m_FontSampler.Create(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_COMPARE_OP_NEVER);

            // Texture
            m_FontTexture.Create(
                fontData,
                uploadSize,
                { static_cast<std::uint32_t>(texWidth), static_cast<std::uint32_t>(texHeight) },
                VK_IMAGE_USAGE_SAMPLED_BIT,
                &m_FontSampler);

            // Pipeline layout
            m_PipelineLayout.AddBinding(0u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u, VK_SHADER_STAGE_FRAGMENT_BIT);
            m_PipelineLayout.AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(m_PushConstantData), 0u);
            m_PipelineLayout.CreateSet();
            m_PipelineLayout.Create();

            // Descriptor set
            auto index{ m_DescriptorSet.GetSize() };
            m_DescriptorSet.EmplaceBack();
            m_DescriptorSet[index].Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, m_ImageViewCount);
            m_DescriptorSet[index].AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            m_DescriptorSet[index].Create(m_PipelineLayout.GetSetLayout());

            m_DescriptorSet[index].Update(
                m_FontTexture.GetDescriptor(),
                0u,                                         // dstSetIndex
                0u,                                         // dstBinding
                0u,                                         // arrayElement
                1u,                                         // array count
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // type

            // // Save pointer to descriptor set
            io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(&m_DescriptorSet[index]));

            // Vertex layout
            VertexLayout vertexLayout;
            vertexLayout.AddBinding(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
            vertexLayout.AddAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, ADH_OFFSET(ImDrawVert, pos));
            vertexLayout.AddAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, ADH_OFFSET(ImDrawVert, uv));
            vertexLayout.AddAttribute(2, 0, VK_FORMAT_R8G8B8A8_UNORM, ADH_OFFSET(ImDrawVert, col));
            vertexLayout.Create();

            // Shader
            Shader shader("ui.vert", "ui.frag");

            // Graphics pipeline
            m_GraphicsPipeline.Create(
                shader,
                vertexLayout,
                m_PipelineLayout,
                renderPass,
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                VK_CULL_MODE_NONE,
                VK_FRONT_FACE_COUNTER_CLOCKWISE,
                VK_SAMPLE_COUNT_1_BIT,
                VK_FALSE,
                0.0f,
                VK_TRUE);

            // Set dpi
            // io.FontGlobalScale *= 2;
        }

        [[nodiscard]] void* VulkanImGui::AddTexture(const std::string& name, VkImageView imageView, const vk::Sampler& sampler) {
            ADH_THROW(m_DescriptorSet.GetSize() < maxTextures, "Too many ImGui textures!");
            auto index{ m_DescriptorSet.GetSize() };
            m_DescriptorSet.EmplaceBack();

            m_DescriptorSet[index].Initialize(VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, m_ImageViewCount);
            m_DescriptorSet[index].AddPool(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            m_DescriptorSet[index].Create(m_PipelineLayout.GetSetLayout());

            VkDescriptorImageInfo texture;
            texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texture.imageView   = imageView;
            texture.sampler     = sampler;

            m_DescriptorSet[index].Update(
                texture,
                0u,                                         // dstSetIndex
                0u,                                         // dstBinding
                0u,                                         // arrayElement
                1u,                                         // array count
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // type

            m_DescriptorIndices.emplace(name, index);

            return static_cast<void*>(&m_DescriptorSet[index]);
        }

        void VulkanImGui::UpdateTexture(const std::string& name, VkImageView imageView, const vk::Sampler& sampler) {
            VkDescriptorImageInfo texture;
            texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texture.imageView   = imageView;
            texture.sampler     = sampler;

            m_DescriptorSet[m_DescriptorIndices[name]].Update(
                texture,
                0u,                                         // dstSetIndex
                0u,                                         // dstBinding
                0u,                                         // arrayElement
                1u,                                         // array count
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // type
        }

        void VulkanImGui::Draw(VkCommandBuffer commandBuffer, std::uint32_t imageIndex) {
            ImDrawData* drawData{ ImGui::GetDrawData() };
            int fbWidth{ static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x) };
            int fbHeight{ static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y) };

            if (fbWidth <= 0 || fbHeight <= 0) {
                return;
            }

            if (drawData->TotalVtxCount > 0) {
                VkDeviceSize vertexSize{ static_cast<VkDeviceSize>(drawData->TotalVtxCount * sizeof(ImDrawVert)) };
                VkDeviceSize indexSize{ static_cast<VkDeviceSize>(drawData->TotalIdxCount * sizeof(ImDrawIdx)) };

                if (m_VertexBuffer[imageIndex].count < vertexSize || m_VertexBuffer[imageIndex].buffer == VK_NULL_HANDLE) {
                    m_VertexBuffer[imageIndex].buffer.Destroy();

                    m_VertexBuffer[imageIndex].buffer.Create(vertexSize, 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
                    m_VertexBuffer[imageIndex].buffer.Map(nullptr, m_VertexBuffer[imageIndex].mappedPtr);

                    m_VertexBuffer[imageIndex].count = vertexSize;
                }

                if (m_IndexBuffer[imageIndex].count < indexSize || m_IndexBuffer[imageIndex].buffer == VK_NULL_HANDLE) {
                    m_IndexBuffer[imageIndex].buffer.Destroy();

                    m_IndexBuffer[imageIndex].buffer.Create(indexSize, 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
                    m_IndexBuffer[imageIndex].buffer.Map(nullptr, m_IndexBuffer[imageIndex].mappedPtr);

                    m_IndexBuffer[imageIndex].count = indexSize;
                }

                ImDrawVert* vertexDest{ static_cast<ImDrawVert*>(m_VertexBuffer[imageIndex].mappedPtr) };
                ImDrawIdx* indexDest{ static_cast<ImDrawIdx*>(m_IndexBuffer[imageIndex].mappedPtr) };

                for (const ImDrawList* cmdList : drawData->CmdLists) {
                    memcpy(vertexDest, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
                    memcpy(indexDest, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vertexDest += cmdList->VtxBuffer.Size;
                    indexDest += cmdList->IdxBuffer.Size;
                }
            }

            m_Viewport.Update(static_cast<float>(fbWidth), static_cast<float>(fbHeight), false);
            m_Viewport.Set(commandBuffer);

            m_PushConstantData.scale[0]     = 2.0f / drawData->DisplaySize.x;
            m_PushConstantData.scale[1]     = 2.0f / drawData->DisplaySize.y;
            m_PushConstantData.translate[0] = -1.0f - drawData->DisplayPos.x * m_PushConstantData.scale[0];
            m_PushConstantData.translate[1] = -1.0f - drawData->DisplayPos.y * m_PushConstantData.scale[1];
            vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(m_PushConstantData), &m_PushConstantData);

            m_GraphicsPipeline.Bind(commandBuffer);
            vkCmdSetDepthBias(commandBuffer, 0.0f, 0.0f, 0.0f);

            if (drawData->TotalVtxCount > 0) {
                VkBuffer vertexBuffer[1]{ m_VertexBuffer[imageIndex].buffer };
                VkDeviceSize vertexOffset[1]{};

                vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, vertexBuffer, vertexOffset);
                vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer[imageIndex].buffer, 0, sizeof(ImDrawIdx) == 2u ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
            }

            ImVec2 clipOff{ drawData->DisplayPos };
            ImVec2 clipScale{ drawData->FramebufferScale };
            int globalVertexOffset{};
            int globalIndexOffset{};

            for (const ImDrawList* cmdList : drawData->CmdLists) {
                for (int j{}; j != cmdList->CmdBuffer.Size; ++j) {
                    const ImDrawCmd* pcmd{ &cmdList->CmdBuffer[j] };

                    ImVec4 clipRect;
                    clipRect.x = (pcmd->ClipRect.x - clipOff.x) * clipScale.x;
                    clipRect.y = (pcmd->ClipRect.y - clipOff.y) * clipScale.y;
                    clipRect.z = (pcmd->ClipRect.z - clipOff.x) * clipScale.x;
                    clipRect.w = (pcmd->ClipRect.w - clipOff.y) * clipScale.y;

                    if (clipRect.x < static_cast<float>(fbWidth) && clipRect.y < static_cast<float>(fbHeight) && clipRect.z >= 0.0f && clipRect.w >= 0.0f) {
                        if (clipRect.x < 0.0f) {
                            clipRect.x = 0.0f;
                        }

                        if (clipRect.y < 0.0f) {
                            clipRect.y = 0.0f;
                        }

                        DescriptorSet* descriptorSet{ reinterpret_cast<DescriptorSet*>(pcmd->GetTexID()) };
                        VkDescriptorSet descSet[]{ descriptorSet->GetSet(0, imageIndex) };
                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0u, 1u, descSet, 0u, nullptr);

                        m_Scissor.Update(
                            static_cast<std::uint32_t>(clipRect.z - clipRect.x),
                            static_cast<std::uint32_t>(clipRect.w - clipRect.y),
                            static_cast<std::int32_t>(clipRect.x),
                            static_cast<std::int32_t>(clipRect.y));
                        m_Scissor.Set(commandBuffer);

                        vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, pcmd->IdxOffset + globalIndexOffset, pcmd->VtxOffset + globalVertexOffset, 0u);
                    }
                }

                globalIndexOffset += cmdList->IdxBuffer.Size;
                globalVertexOffset += cmdList->VtxBuffer.Size;
            }
        }
    } // namespace vk
} // namespace adh
