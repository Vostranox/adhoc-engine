#include "Shader.hpp"
#include "Context.hpp"
#include "vulkan/vulkan_core.h"
#include <Std/File.hpp>

namespace adh {
    namespace vk {
        Shader::Shader() noexcept {
        }

        Shader::Shader(Shader&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Shader& Shader::operator=(Shader&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));
            return *this;
        }

        Shader::~Shader() {
            Clear();
        }

        VkPipelineShaderStageCreateInfo& Shader::Get() noexcept {
            return m_ShaderStageCreateInfo[0];
        }

        const VkPipelineShaderStageCreateInfo& Shader::Get() const noexcept {
            return m_ShaderStageCreateInfo[0];
        }

        std::size_t Shader::GetSize() const noexcept {
            return m_ShaderStageCreateInfo.GetSize();
        }

        void Shader::Destroy() noexcept {
            Clear();
        }

        VkShaderModule Shader::LoadFile(const std::string& shader) {
            ADH_THROW(!shader.empty(), "Null shader file path!");
            auto context{ Context::Get() };
            std::string path{ Context::Get()->GetDataDirectory() + "Resources/Shaders/" + shader + ".spv" };
            File file(path.data(), File::Mode::ReadBinary);
            auto size{ file.GetSize() };
            Array<char> buffer;
            buffer.Resize(size);
            file.Read(buffer.GetData(), size);
            auto info{ initializers::ShaderModuleCreateInfo(size, buffer.GetData()) };
            VkShaderModule returnValue;
            ADH_THROW(vkCreateShaderModule(context->GetDevice(), &info, nullptr, &returnValue) == VK_SUCCESS,
                      "Failed to create shader module!");
            return returnValue;
        }

        void Shader::MoveConstruct(Shader&& rhs) noexcept {
            m_ShaderModules         = Move(rhs.m_ShaderModules);
            m_ShaderStageCreateInfo = Move(rhs.m_ShaderStageCreateInfo);
            m_SpecializationEntry   = rhs.m_SpecializationEntry;
            m_SpecializationInfo    = rhs.m_SpecializationInfo;
            m_EnablePCF             = rhs.m_EnablePCF;
        }

        void Shader::Clear() noexcept {
            auto device{ Context::Get()->GetDevice() };
            vkDeviceWaitIdle(device);
            for (std::size_t i{}; i != m_ShaderModules.GetSize(); ++i) {
                vkDestroyShaderModule(device, m_ShaderModules[i], nullptr);
                m_ShaderModules[i] = VK_NULL_HANDLE;
            }
        }
    } // namespace vk
} // namespace adh
