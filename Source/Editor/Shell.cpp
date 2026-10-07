#include "Shell.hpp"

#include <Event/Event.hpp>
#include <Utf8.hpp>

#include <ImGui/imgui.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#if defined(ADH_WINDOWS)
#    include <Windows.h>
#    include <shellapi.h>
#else
#    include <cerrno>
#    include <spawn.h>
#    include <sys/wait.h>
#    include <thread>
extern char** environ;
#endif

namespace adh {
    namespace shell {
        static void LogError(const std::filesystem::path& path, const std::string& what, const std::string& reason) {
            const std::string message{ "[" + ToUtf8(path) + "] Failed to " + what + ": " + reason + "\n" };
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, message.c_str());
        }

        struct DialogState {
            bool isShown{};
            bool openPopup{};
            bool isClosed{};
            FileDialog kind{};
            std::filesystem::path directory;
            std::filesystem::path chosen;
            std::array<char, 4096> location{};
            std::array<char, 1024> filename{};
            std::string error;
            std::function<void(const std::filesystem::path&)> onChosen;
        };

        static DialogState dialogState;

        template <std::size_t N>
        static void SetText(std::array<char, N>& buffer, const std::string& text) {
            std::snprintf(buffer.data(), buffer.size(), "%s", text.c_str());
        }

        static void SetDirectory(const std::filesystem::path& directory) {
            auto& state{ dialogState };
            std::error_code error;
            if (!std::filesystem::is_directory(directory, error)) {
                state.error = error ? error.message() : "The folder does not exist.";
                return;
            }
            state.directory = std::filesystem::absolute(directory, error).lexically_normal();
            if (error) {
                state.error = error.message();
                return;
            }
            SetText(state.location, ToUtf8(state.directory));
            state.error.clear();
        }

        static void Choose(const std::filesystem::path& path) {
            dialogState.chosen   = path;
            dialogState.isClosed = true;
            ImGui::CloseCurrentPopup();
        }

        void Open(const std::filesystem::path& path) {
            std::error_code error;
            const auto absolute{ std::filesystem::absolute(path, error) };
            if (error) {
                LogError(path, "open", error.message());
                return;
            }
#if defined(ADH_WINDOWS)
            const auto result{ reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", absolute.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) };
            if (result <= 32) {
                LogError(path, "open", "Desktop error " + std::to_string(result));
            }
#else
#    if defined(ADH_APPLE)
            const char* command{ "/usr/bin/open" };
#    else
            const char* command{ "xdg-open" };
#    endif
            std::string text{ ToUtf8(absolute) };
            char* args[]{ const_cast<char*>(command), text.data(), nullptr };
            pid_t child{};
            const int result{ posix_spawnp(&child, command, nullptr, nullptr, args, environ) };
            if (result != 0) {
                LogError(path, "open", std::generic_category().message(result));
                return;
            }
            std::thread([child] {
                while (waitpid(child, nullptr, 0) < 0 && errno == EINTR) {
                }
            }).detach();
#endif
        }

        bool CopyIfMissing(const std::filesystem::path& from, const std::filesystem::path& to) {
            std::error_code error;
            std::filesystem::copy_file(from, to, std::filesystem::copy_options::skip_existing, error);
            if (error) {
                LogError(to, "copy " + ToUtf8(from), error.message());
                return false;
            }
            return true;
        }

        void Delete(const std::filesystem::path& path) {
            std::error_code error;
            std::filesystem::remove(path, error);
            if (error) {
                LogError(path, "delete", error.message());
            }
        }

        void ShowFileDialog(FileDialog dialog, const std::filesystem::path& location, std::function<void(const std::filesystem::path&)> onChosen) {
            auto& state{ dialogState };
            if (state.isShown) {
                return;
            }
            state         = {};
            state.isShown = state.openPopup = true;
            state.kind                      = dialog;
            state.onChosen                  = std::move(onChosen);
            std::error_code error;
            auto start{ std::filesystem::absolute(location, error) };
            if (!std::filesystem::is_directory(start, error)) {
                SetText(state.filename, ToUtf8(start.filename()));
                start = start.parent_path();
            }
            if (!std::filesystem::is_directory(start, error)) {
                start = std::filesystem::current_path(error);
            }
            SetDirectory(start);
        }

        void DrawFileDialog() {
            auto& state{ dialogState };
            if (!state.isShown || state.isClosed) {
                return;
            }
            if (state.openPopup) {
                ImGui::OpenPopup("Choose scene###FileDialog");
                state.openPopup = false;
            }
            ImGui::SetNextWindowSize(ImVec2(680, 480), ImGuiCond_FirstUseEver);
            bool open{ true };
            if (ImGui::BeginPopupModal("Choose scene###FileDialog", &open)) {
                if (ImGui::Button("Up")) {
                    SetDirectory(state.directory.parent_path());
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##Folder", state.location.data(), state.location.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    SetDirectory(PathFromUtf8(state.location.data()));
                }
                ImGui::BeginChild("Files", ImVec2(0, -110), ImGuiChildFlags_Borders);
                std::error_code error;
                std::vector<std::filesystem::directory_entry> entries;
                for (std::filesystem::directory_iterator it(state.directory, error), end; !error && it != end; it.increment(error)) {
                    entries.push_back(*it);
                }
                if (error) {
                    state.error = error.message();
                }
                std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) { return a.path().filename() < b.path().filename(); });
                for (const auto& entry : entries) {
                    const bool folder{ entry.is_directory(error) };
                    if (!folder && entry.path().extension() != ".scene") {
                        continue;
                    }
                    const auto filename{ ToUtf8(entry.path().filename()) };
                    const auto label{ folder ? "[Folder] " + filename : filename };
                    if (ImGui::Selectable(label.c_str(), filename == state.filename.data(), ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (folder) {
                            SetDirectory(entry.path());
                            break;
                        }
                        SetText(state.filename, filename);
                        if (state.kind == FileDialog::eOpen && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            Choose(entry.path());
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::InputText("File name", state.filename.data(), state.filename.size());
                ImGui::TextUnformatted(state.error.empty() ? "Scenes" : state.error.c_str());
                if (ImGui::Button(state.kind == FileDialog::eOpen ? "Open" : "Save")) {
                    auto chosen{ (state.directory / PathFromUtf8(state.filename.data())).lexically_normal() };
                    if (!state.filename[0]) {
                        state.error = "Enter a file name.";
                    } else if (std::filesystem::is_directory(chosen, error)) {
                        SetDirectory(chosen);
                    } else if (state.kind == FileDialog::eOpen) {
                        if (std::filesystem::is_regular_file(chosen, error)) {
                            Choose(chosen);
                        } else {
                            state.error = "The file does not exist.";
                        }
                    } else {
                        if (chosen.extension().empty()) {
                            chosen += ".scene";
                        }
                        if (std::filesystem::exists(chosen, error)) {
                            state.chosen = chosen;
                            ImGui::OpenPopup("Replace file?");
                        } else {
                            Choose(chosen);
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    Choose({});
                }
                if (ImGui::BeginPopupModal("Replace file?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextUnformatted("A file with this name already exists. Replace it?");
                    if (ImGui::Button("Replace")) {
                        state.isClosed = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        state.chosen.clear();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                    if (state.isClosed) {
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }
            if (!open) {
                state.chosen.clear();
                state.isClosed = true;
            }
        }

        void PollFileDialog() {
            auto& state{ dialogState };
            if (!state.isClosed) {
                return;
            }
            auto chosen{ std::move(state.chosen) };
            auto callback{ std::move(state.onChosen) };
            state = {};
            if (!chosen.empty()) {
                callback(chosen);
            }
        }

        bool IsFileDialogShown() {
            return dialogState.isShown;
        }
    } // namespace shell
} // namespace adh
