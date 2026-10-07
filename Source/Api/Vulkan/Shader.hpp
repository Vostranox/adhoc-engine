#pragma once
#include "Initializers.hpp"
#include <Std/Array.hpp>
#include <Utility.hpp>

#include <string>
#include <vulkan/vulkan.h>

// TODO: Add --target-env vulkan1.2 to shaders for ray tracing
namespace adh {
    namespace vk {
        class Shader {
          public:
            Shader() noexcept;

            template <typename... Args>
            inline Shader(Args&&... args);

            Shader(const Shader&) = delete;

            Shader& operator=(const Shader&) = delete;

            Shader(Shader&& rhs) noexcept;

            Shader& operator=(Shader&& rhs) noexcept;

            ~Shader();

            template <typename Head, typename... Args>
            inline void Create(Head&& head, Args&&... args);

            VkPipelineShaderStageCreateInfo& Get() noexcept;

            const VkPipelineShaderStageCreateInfo& Get() const noexcept;

            std::size_t GetSize() const noexcept;

            void Destroy() noexcept;

          private:
            [[nodiscard]] VkShaderModule LoadFile(const std::string& shader);

            void MoveConstruct(Shader&& rhs) noexcept;

            void Clear() noexcept;

          private:
            Array<VkShaderModule> m_ShaderModules;
            Array<VkPipelineShaderStageCreateInfo> m_ShaderStageCreateInfo;
            VkSpecializationMapEntry m_SpecializationEntry{};
            VkSpecializationInfo m_SpecializationInfo{};
            std::uint32_t m_EnablePCF{ 1u };
        };
    } // namespace vk
} // namespace adh

#include "Shader.inl"
