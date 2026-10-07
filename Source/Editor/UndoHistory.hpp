#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace adh {
    struct SceneSnapshot {
        std::string text;
        std::optional<std::size_t> selected;
        std::uint64_t id{};
    };

    class UndoHistory {
      public:
        static constexpr std::size_t maxEdits{ 64 };

      public:
        void Reset(std::string text);

        void Record(std::string name, std::optional<std::size_t> selectedBefore, std::string text, std::optional<std::size_t> selectedAfter);

        bool CanUndo() const noexcept;

        bool CanRedo() const noexcept;

        const std::string& GetUndoName() const noexcept;

        const std::string& GetRedoName() const noexcept;

        const SceneSnapshot& Undo();

        const SceneSnapshot& Redo();

        void MarkSaved() noexcept;

        bool HasUnsavedChanges() const noexcept;

      private:
        struct Edit {
            std::string name;
            SceneSnapshot scene;
        };

      private:
        std::deque<Edit> m_Undo;
        std::vector<Edit> m_Redo;
        SceneSnapshot m_Current;
        std::uint64_t m_LastId{};
        std::uint64_t m_SavedId{};
    };
} // namespace adh
