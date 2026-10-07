#include "Widgets.hpp"
#include "DragDrop.hpp"
#include "IconFontCppHeaders/IconFontAwesome5.hpp"
#include "Shell.hpp"
#include "Theme.hpp"

#include <ImGui/imgui_internal.h>

namespace adh {
    namespace widgets {
        static ImVec4 Lighter(const ImVec4& colour, float amount) {
            return ImLerp(colour, ImVec4{ 1.0f, 1.0f, 1.0f, colour.w }, amount);
        }

        bool ToggleButton(const char* label, const char* tooltip, bool isOn, bool isEnabled, float width) {
            if (isOn) {
                ImGui::PushStyleColor(ImGuiCol_Button, theme::accent);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme::accentHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme::accentActive);
            }
            ImGui::BeginDisabled(!isEnabled);
            const bool clicked{ ImGui::Button(label, ImVec2{ width, 0.0f }) };
            ImGui::EndDisabled();
            if (isOn) {
                ImGui::PopStyleColor(3);
            }
            ImGui::SetItemTooltip("%s", tooltip);
            return clicked;
        }

        void SearchField(ImGuiTextFilter& filter) {
            ImGui::PushItemFlag(ImGuiItemFlags_NoMarkEdited, true);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputTextWithHint("##Search", ICON_FA_SEARCH "  Search", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf))) {
                filter.Build();
            }
            ImGui::PopItemFlag();
        }

        bool BeginProperties(const char* id) {
            const float labelWidth{ 0.36f * ImGui::GetContentRegionAvail().x };
            if (!ImGui::BeginTable(id, 2)) {
                return false;
            }
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            return true;
        }

        void EndProperties() {
            ImGui::EndTable();
        }

        void Property(const char* label) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            const bool isCut{ ImGui::CalcTextSize(label).x > ImGui::GetContentRegionAvail().x };
            ImGui::TextUnformatted(label);
            if (isCut) {
                ImGui::SetItemTooltip("%s", label);
            }
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
        }

        bool DragFloat(const char* label, float& value, float speed, float min, float max, const char* format, ImGuiSliderFlags flags) {
            Property(label);
            ImGui::PushID(label);
            const bool changed{ ImGui::DragFloat("##Value", &value, speed, min, max, format, flags) };
            ImGui::PopID();
            return changed;
        }

        bool SliderFloat(const char* label, float& value, float min, float max, const char* format, ImGuiSliderFlags flags) {
            Property(label);
            ImGui::PushID(label);
            const bool changed{ ImGui::SliderFloat("##Value", &value, min, max, format, flags) };
            ImGui::PopID();
            return changed;
        }

        bool SliderInt(const char* label, int& value, int min, int max, ImGuiSliderFlags flags) {
            Property(label);
            ImGui::PushID(label);
            const bool changed{ ImGui::SliderInt("##Value", &value, min, max, "%d", flags) };
            ImGui::PopID();
            return changed;
        }

        bool Checkbox(const char* label, bool& value) {
            Property(label);
            ImGui::PushID(label);
            const bool changed{ ImGui::Checkbox("##Value", &value) };
            ImGui::PopID();
            return changed;
        }

        bool ColorEdit3(const char* label, float* rgb, ImGuiColorEditFlags flags) {
            Property(label);
            ImGui::PushID(label);
            const bool changed{ ImGui::ColorEdit3("##Value", rgb, flags) };
            ImGui::PopID();
            return changed;
        }

        void Vector3(const char* label, Vector3D& value, float resetValue, float speed) {
            Property(label);
            ImGui::PushID(label);

            constexpr const char* letters[3]{ "X", "Y", "Z" };
            constexpr ImVec4 colours[3]{ theme::axisX, theme::axisY, theme::axisZ };
            const float gap{ ImGui::GetStyle().ItemInnerSpacing.x };
            const float partWidth{ (ImGui::GetContentRegionAvail().x - 2.0f * gap) / 3.0f };
            const float letterWidth{ ImGui::GetFrameHeight() * 0.75f };
            for (std::size_t axis{}; axis != 3; ++axis) {
                if (axis != 0) {
                    ImGui::SameLine(0.0f, gap);
                }
                ImGui::PushID(letters[axis]);

                ImGui::PushStyleColor(ImGuiCol_Button, colours[axis]);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Lighter(colours[axis], 0.2f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, Lighter(colours[axis], 0.1f));
                ImGui::PushFont(theme::GetFonts().bold, 0.0f);
                if (ImGui::Button(letters[axis], ImVec2{ letterWidth, 0.0f }) && value[axis] != resetValue) {
                    value[axis] = resetValue;
                    ImGui::MarkItemEdited(ImGui::GetItemID());
                }
                ImGui::PopFont();
                ImGui::PopStyleColor(3);
                ImGui::SetItemTooltip("Set %s to %g", letters[axis], static_cast<double>(resetValue));

                ImGui::SameLine(0.0f, 0.0f);
                ImGui::SetNextItemWidth(partWidth - letterWidth);
                ImGui::DragFloat("##Value", &value[axis], speed, 0.0f, 0.0f, "%.2f");
                ImGui::PopID();
            }

            ImGui::PopID();
        }

        std::optional<std::filesystem::path> AssetField(const char* label, const std::string& name, const std::filesystem::path& file,
                                                        bool canOpen, std::initializer_list<std::string_view> extensions, bool* cleared) {
            Property(label);
            ImGui::PushID(label);

            const float clearWidth{ cleared ? ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f };
            const float width{ ImGui::GetContentRegionAvail().x - clearWidth };
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });
            ImGui::Button(name.c_str(), ImVec2{ width, 0.0f });
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            bool open{ canOpen && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) };
            if (canOpen && ImGui::BeginPopupContextItem("Menu")) {
                if (ImGui::MenuItem("Open")) {
                    open = true;
                }
                ImGui::EndPopup();
            }
            if (open) {
                shell::Open(file);
            }
            std::optional<std::filesystem::path> dropped{ AcceptAssetPath(extensions) };

            if (cleared) {
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                *cleared = ImGui::Button(ICON_FA_TIMES, ImVec2{ ImGui::GetFrameHeight(), 0.0f });
                ImGui::SetItemTooltip("Clear");
            }

            ImGui::PopID();
            return dropped;
        }
    } // namespace widgets
} // namespace adh
