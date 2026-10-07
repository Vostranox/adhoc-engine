#pragma once
#include <filesystem>
#include <functional>

namespace adh {
    namespace shell {
        enum class FileDialog {
            eOpen,
            eSave
        };

        void Open(const std::filesystem::path& path);

        bool CopyIfMissing(const std::filesystem::path& from, const std::filesystem::path& to);

        void Delete(const std::filesystem::path& path);

        void ShowFileDialog(FileDialog dialog, const std::filesystem::path& location, std::function<void(const std::filesystem::path&)> onChosen);

        void DrawFileDialog();

        void PollFileDialog();

        bool IsFileDialogShown();
    } // namespace shell
} // namespace adh
