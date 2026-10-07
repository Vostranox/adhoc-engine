#include "AssetPanel.hpp"
#include "../DragDrop.hpp"
#include "../EditorActions.hpp"
#include "../EditorContext.hpp"
#include "../Shell.hpp"
#include "../Theme.hpp"

#include <Utf8.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <algorithm>
#include <cctype>
#include <system_error>
#include <utility>

namespace adh {
    static const char* GetIcon(const std::filesystem::path& path, bool isFolder) {
        if (isFolder) {
            return ICON_FA_FOLDER;
        }
        std::string extension{ path.extension().string() };
        std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (extension == ".scene") {
            return ICON_FA_CUBES;
        }
        if (extension == ".lua") {
            return ICON_FA_FILE_CODE;
        }
        if (extension == ".obj" || extension == ".fbx" || extension == ".glb" || extension == ".gltf" || extension == ".ply") {
            return ICON_FA_CUBE;
        }
        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".tga" || extension == ".hdr") {
            return ICON_FA_FILE_IMAGE;
        }
        if (extension == ".wav" || extension == ".ogg" || extension == ".mp3" || extension == ".flac") {
            return ICON_FA_FILE_AUDIO;
        }
        if (extension == ".ttf" || extension == ".otf") {
            return ICON_FA_FONT;
        }
        return ICON_FA_FILE;
    }

    static bool IsBefore(const std::string& a, const std::string& b) {
        return std::ranges::lexicographical_compare(a, b, [](unsigned char x, unsigned char y) {
            return std::tolower(x) < std::tolower(y);
        });
    }

    AssetPanel::AssetPanel() noexcept : Panel{ "Assets" } {
    }

    void AssetPanel::OnImGui(EditorContext& context) {
        constexpr double listInterval{ 1.0 };
        if (m_ListTime < 0.0 || ImGui::GetTime() - m_ListTime >= listInterval) {
            List(context);
        }

        ImGui::BeginDisabled(m_Folder.empty());
        if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
            m_Folder   = m_Folder.parent_path();
            m_ListTime = -1.0;
        }
        ImGui::EndDisabled();
        ImGui::SetItemTooltip("Back to the folder above");
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        const std::filesystem::path shown{ context.paths.assets.filename() };
        ImGui::TextDisabled("%s", ToUtf8(m_Folder.empty() ? shown : shown / m_Folder).c_str());

        ImGui::BeginChild("Items");
        const float tileWidth{ 2.0f * theme::GetFonts().largeIcons->LegacySize };
        const float spacing{ ImGui::GetStyle().ItemSpacing.x };
        const int columns{ std::max(1, static_cast<int>((ImGui::GetContentRegionAvail().x + spacing) / (tileWidth + spacing))) };
        std::filesystem::path toDelete;
        for (std::size_t i{}; i != m_Items.size(); ++i) {
            if (i % static_cast<std::size_t>(columns) != 0) {
                ImGui::SameLine();
            }
            if (DrawItem(context, m_Items[i], tileWidth)) {
                toDelete = m_Items[i].path;
            }
        }

        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_Selected.clear();
        }
        ImGui::EndChild();

        if (!toDelete.empty()) {
            m_ToDelete = std::move(toDelete);
            ImGui::OpenPopup("Delete?");
        }
        DrawDeleteConfirmation();
    }

    void AssetPanel::List(EditorContext& context) {
        m_ListTime = ImGui::GetTime();
        m_Items.clear();

        std::error_code error;
        std::filesystem::directory_iterator entry{ context.paths.assets / m_Folder, error };
        if (error) {
            if (!m_Folder.empty()) {
                m_Folder   = m_Folder.parent_path();
                m_ListTime = -1.0;
            }
            return;
        }
        for (; !error && entry != std::filesystem::directory_iterator{}; entry.increment(error)) {
            const std::filesystem::path& path{ entry->path() };
            const std::string name{ ToUtf8(path.filename()) };
            if (!name.starts_with('.')) {
                std::error_code folderError;
                m_Items.push_back(Item{ path, name, entry->is_directory(folderError) });
            }
        }

        std::ranges::sort(m_Items, [](const Item& a, const Item& b) {
            if (a.isFolder != b.isFolder) {
                return a.isFolder;
            }
            return IsBefore(a.name, b.name);
        });
    }

    bool AssetPanel::DrawItem(EditorContext& context, const Item& item, float tileWidth) {
        ImGui::PushID(item.name.c_str());

        ImFont* iconFont{ theme::GetFonts().largeIcons };
        const float padding{ ImGui::GetStyle().FramePadding.y };
        const float iconSize{ iconFont->LegacySize };
        const ImVec2 min{ ImGui::GetCursorScreenPos() };
        const ImVec2 size{ tileWidth, padding + iconSize + padding + ImGui::GetFontSize() + padding };
        const bool isSelected{ item.path == m_Selected };
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, theme::accent);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, theme::accentHovered);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, theme::accentActive);
        }
        if (ImGui::Selectable("##Tile", isSelected, ImGuiSelectableFlags_AllowDoubleClick, size)) {
            m_Selected = item.path;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                Open(context, item);
            }
        }
        if (isSelected) {
            ImGui::PopStyleColor(3);
        }
        ImGui::SetItemTooltip("%s", item.name.c_str());
        if (!item.isFolder) {
            DragAssetPath(item.path);
        }

        bool isDeleteChosen{};
        if (ImGui::BeginPopupContextItem("Menu")) {
            m_Selected = item.path;
            if (ImGui::MenuItem("Open", nullptr, false, item.path.extension() != ".scene" || actions::CanChangeFiles(context))) {
                Open(context, item);
            }
            isDeleteChosen = ImGui::MenuItem("Delete...");
            ImGui::EndPopup();
        }

        ImDrawList* drawList{ ImGui::GetWindowDrawList() };
        const char* icon{ GetIcon(item.path, item.isFolder) };
        const float iconWidth{ iconFont->CalcTextSizeA(iconSize, FLT_MAX, 0.0f, icon).x };
        const ImVec4 iconColour{ item.isFolder ? theme::folderIcon : theme::textDim };
        drawList->AddText(iconFont, iconSize, ImVec2{ min.x + (tileWidth - iconWidth) / 2.0f, min.y + padding }, ImGui::GetColorU32(iconColour), icon);

        const float nameY{ min.y + padding + iconSize + padding };
        const float nameWidth{ ImGui::CalcTextSize(item.name.c_str()).x };
        const float maxWidth{ tileWidth - 2.0f * padding };
        if (nameWidth <= maxWidth) {
            drawList->AddText(ImVec2{ min.x + (tileWidth - nameWidth) / 2.0f, nameY }, ImGui::GetColorU32(ImGuiCol_Text), item.name.c_str());
        } else {
            const ImVec2 nameMin{ min.x + padding, nameY };
            const ImVec2 nameMax{ min.x + tileWidth - padding, nameY + ImGui::GetFontSize() };
            ImGui::RenderTextEllipsis(drawList, nameMin, nameMax, nameMax.x, item.name.c_str(), nullptr, nullptr);
        }

        ImGui::PopID();
        return isDeleteChosen;
    }

    void AssetPanel::Open(EditorContext& context, const Item& item) {
        if (item.isFolder) {
            m_Folder /= item.path.filename();
            m_ListTime = -1.0;
        } else if (item.path.extension() == ".scene") {
            actions::OpenSceneFile(context, item.path);
        } else {
            shell::Open(item.path);
        }
    }

    void AssetPanel::DrawDeleteConfirmation() {
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Delete %s? This can't be undone.", ToUtf8(m_ToDelete.filename()).c_str());
            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                shell::Delete(m_ToDelete);
                m_ToDelete.clear();
                m_ListTime = -1.0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                m_ToDelete.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
} // namespace adh
