#include "DragDrop.hpp"

#include <Utf8.hpp>

#include <ImGui/imgui.h>

#include <algorithm>
#include <string>

namespace adh {
    static constexpr const char* assetPathPayload{ "ASSET_PATH" };

    void DragAssetPath(const std::filesystem::path& path) {
        if (ImGui::BeginDragDropSource()) {
            const std::string text{ ToUtf8(path) };
            ImGui::SetDragDropPayload(assetPathPayload, text.c_str(), text.size() + 1u);
            ImGui::EndDragDropSource();
        }
    }

    std::optional<std::filesystem::path> AcceptAssetPath(std::initializer_list<std::string_view> extensions) {
        std::optional<std::filesystem::path> dropped;
        if (ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload{ ImGui::GetDragDropPayload() };
            if (payload && payload->IsDataType(assetPathPayload)) {
                std::filesystem::path path{ PathFromUtf8(static_cast<const char*>(payload->Data)) };
                if (std::ranges::find(extensions, path.extension().string()) != extensions.end() && ImGui::AcceptDragDropPayload(assetPathPayload)) {
                    dropped = std::move(path);
                }
            }
            ImGui::EndDragDropTarget();
        }
        return dropped;
    }
} // namespace adh
