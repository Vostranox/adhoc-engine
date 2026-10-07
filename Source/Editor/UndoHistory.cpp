#include "UndoHistory.hpp"

#include <utility>

namespace adh {
    void UndoHistory::Reset(std::string text) {
        m_Undo.clear();
        m_Redo.clear();
        m_Current = SceneSnapshot{ std::move(text), std::nullopt, ++m_LastId };
        m_SavedId = m_Current.id;
    }

    void UndoHistory::Record(std::string name, std::optional<std::size_t> selectedBefore, std::string text, std::optional<std::size_t> selectedAfter) {
        if (text == m_Current.text) {
            return;
        }
        m_Current.selected = selectedBefore;
        m_Undo.push_back(Edit{ std::move(name), std::move(m_Current) });
        if (m_Undo.size() > maxEdits) {
            m_Undo.pop_front();
        }
        m_Current = SceneSnapshot{ std::move(text), selectedAfter, ++m_LastId };
        m_Redo.clear();
    }

    bool UndoHistory::CanUndo() const noexcept {
        return !m_Undo.empty();
    }

    bool UndoHistory::CanRedo() const noexcept {
        return !m_Redo.empty();
    }

    const std::string& UndoHistory::GetUndoName() const noexcept {
        return m_Undo.back().name;
    }

    const std::string& UndoHistory::GetRedoName() const noexcept {
        return m_Redo.back().name;
    }

    const SceneSnapshot& UndoHistory::Undo() {
        Edit edit{ std::move(m_Undo.back()) };
        m_Undo.pop_back();
        m_Redo.push_back(Edit{ std::move(edit.name), std::move(m_Current) });
        m_Current = std::move(edit.scene);
        return m_Current;
    }

    const SceneSnapshot& UndoHistory::Redo() {
        Edit edit{ std::move(m_Redo.back()) };
        m_Redo.pop_back();
        m_Undo.push_back(Edit{ std::move(edit.name), std::move(m_Current) });
        m_Current = std::move(edit.scene);
        return m_Current;
    }

    void UndoHistory::MarkSaved() noexcept {
        m_SavedId = m_Current.id;
    }

    bool UndoHistory::HasUnsavedChanges() const noexcept {
        return m_Current.id != m_SavedId;
    }
} // namespace adh
