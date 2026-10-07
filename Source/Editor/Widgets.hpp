#pragma once
#include <Math/Math.hpp>

#include <ImGui/imgui.h>

#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

namespace adh {
    namespace widgets {
        bool ToggleButton(const char* label, const char* tooltip, bool isOn, bool isEnabled = true, float width = 0.0f);

        void SearchField(ImGuiTextFilter& filter);

        bool BeginProperties(const char* id);

        void EndProperties();

        void Property(const char* label);

        bool DragFloat(const char* label, float& value, float speed, float min = 0.0f, float max = 0.0f, const char* format = "%.3f",
                       ImGuiSliderFlags flags = ImGuiSliderFlags_None);

        bool SliderFloat(const char* label, float& value, float min, float max, const char* format = "%.3f",
                         ImGuiSliderFlags flags = ImGuiSliderFlags_None);

        bool SliderInt(const char* label, int& value, int min, int max, ImGuiSliderFlags flags = ImGuiSliderFlags_None);

        bool Checkbox(const char* label, bool& value);

        bool ColorEdit3(const char* label, float* rgb, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None);

        void Vector3(const char* label, Vector3D& value, float resetValue = 0.0f, float speed = 0.1f);

        std::optional<std::filesystem::path> AssetField(const char* label, const std::string& name, const std::filesystem::path& file,
                                                        bool canOpen, std::initializer_list<std::string_view> extensions,
                                                        bool* cleared = nullptr);
    } // namespace widgets
} // namespace adh
