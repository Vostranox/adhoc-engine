#pragma once
#include "Panel.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace adh {
    class AssetPanel : public Panel {
      public:
        AssetPanel() noexcept;

      private:
        struct Item {
            std::filesystem::path path;
            std::string name;
            bool isFolder;
        };

      private:
        void OnImGui(EditorContext& context) override;

        void List(EditorContext& context);

        bool DrawItem(EditorContext& context, const Item& item, float tileWidth);

        void Open(EditorContext& context, const Item& item);

        void DrawDeleteConfirmation();

      private:
        std::filesystem::path m_Folder;
        std::vector<Item> m_Items;
        double m_ListTime{ -1.0 };
        std::filesystem::path m_Selected;
        std::filesystem::path m_ToDelete;
    };
} // namespace adh
