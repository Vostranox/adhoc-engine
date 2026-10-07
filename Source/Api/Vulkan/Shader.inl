namespace adh {
    namespace vk {
        template <typename... Args>
        Shader::Shader(Args&&... args) {
            m_SpecializationEntry = initializers::SpecializationMapEntry(sizeof(std::uint32_t));
            m_SpecializationInfo  = initializers::SpecializationInfo(&m_SpecializationEntry, sizeof(std::uint32_t), &m_EnablePCF);
            Create(Forward<Args>(args)...);
        }

        template <typename Head, typename... Args>
        void Shader::Create(Head&& head, Args&&... args) {
            std::string shader{ head };
            auto pos{ shader.find_last_of(".") };
            auto stage{ shader.substr(pos + 1, std::string::npos) };
            VkShaderStageFlagBits stageFlag{};
            if (stage == "vert") {
                stageFlag = VK_SHADER_STAGE_VERTEX_BIT;
            } else if (stage == "frag") {
                stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
            } else if (stage == "comp") {
                stageFlag = VK_SHADER_STAGE_COMPUTE_BIT;
            } else if (stage == "frag") {
                stageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
            } else if (stage == "rgen") {
                stageFlag = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            } else if (stage == "rchit") {
                stageFlag = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            } else if (stage == "rmiss") {
                stageFlag = VK_SHADER_STAGE_MISS_BIT_KHR;
            }
            ADH_THROW(stageFlag, "Invalid shader type!");
            m_ShaderStageCreateInfo.EmplaceBack(
                initializers::PipelineShaderStageCreateInfo(stageFlag,
                                                            m_ShaderModules.EmplaceBack(LoadFile(shader)),
                                                            &m_SpecializationInfo));
            if constexpr (sizeof...(Args) > 0) {
                Create(args...);
            }
        }
    } // namespace vk
} // namespace adh
