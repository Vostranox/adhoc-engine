#include "ConsolePanel.hpp"
#include "../IconFontCppHeaders/IconFontAwesome5.hpp"
#include "../Theme.hpp"
#include "../Widgets.hpp"

#include <ImGui/imgui.h>

#include <algorithm>
#include <ctime>
#include <string_view>
#include <vector>

namespace adh {
    static std::string GetTimeOfDay() {
        const std::time_t now{ std::time(nullptr) };
        const std::tm* local{ std::localtime(&now) };
        char text[16]{};
        if (local) {
            std::strftime(text, sizeof(text), "%H:%M:%S", local);
        }
        return text;
    }

    static void DrawLine(const ConsoleLine& line, float iconX, float textX) {
        const bool isError{ line.type == EditorLogEvent::Type::eError };
        if (line.isFirst) {
            ImGui::TextColored(theme::textDim, "%s", line.time.c_str());
            ImGui::SameLine(iconX);
            ImGui::PushFont(theme::GetFonts().regular, 0.0f);
            ImGui::TextColored(isError ? theme::error : theme::textDim, "%s", isError ? ICON_FA_TIMES_CIRCLE : ICON_FA_INFO_CIRCLE);
            ImGui::PopFont();
            ImGui::SameLine(textX);
        } else {
            ImGui::SetCursorPosX(textX);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, isError ? theme::error : theme::text);
        ImGui::TextUnformatted(line.text.c_str());
        ImGui::PopStyleColor();
    }

    static std::string ExpandTabs(std::string_view line) {
        std::string text;
        for (const char c : line) {
            if (c == '\t') {
                text += "    ";
            } else {
                text += c;
            }
        }
        return text;
    }

    ConsolePanel::ConsolePanel() : Panel{ "Console" } {
        listener = EventBus().create_subscriber();
        EventBus().subscribe<EditorLogEvent>(listener, this, &ConsolePanel::OnLogEvent);
    }

    ConsolePanel::~ConsolePanel() {
        EventBus().destroy(listener);
    }

    void ConsolePanel::OnImGui(EditorContext&) {
        std::vector<const ConsoleLine*> shown;
        int infoCount{};
        int errorCount{};
        for (const ConsoleLine& line : m_Lines) {
            const bool isError{ line.type == EditorLogEvent::Type::eError };
            if (line.isFirst) {
                ++(isError ? errorCount : infoCount);
            }
            if ((isError ? m_ShowsErrors : m_ShowsInfo) && m_Search.PassFilter(line.text.c_str())) {
                shown.push_back(&line);
            }
        }

        bool isCleared{ ImGui::Button(ICON_FA_TRASH_ALT "  Clear") };
        ImGui::SameLine();
        const std::string info{ ICON_FA_INFO_CIRCLE "  " + std::to_string(infoCount) + "###Info" };
        if (widgets::ToggleButton(info.c_str(), m_ShowsInfo ? "Hide the info" : "Show the info", m_ShowsInfo)) {
            m_ShowsInfo = !m_ShowsInfo;
        }
        ImGui::SameLine();
        const std::string errors{ ICON_FA_TIMES_CIRCLE "  " + std::to_string(errorCount) + "###Errors" };
        if (widgets::ToggleButton(errors.c_str(), m_ShowsErrors ? "Hide the errors" : "Show the errors", m_ShowsErrors)) {
            m_ShowsErrors = !m_ShowsErrors;
        }
        ImGui::SameLine();
        widgets::SearchField(m_Search);

        ImGui::BeginChild("Lines", ImVec2{ 0.0f, 0.0f }, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        const float spacing{ ImGui::GetStyle().ItemSpacing.x };
        const float iconWidth{ std::max(ImGui::CalcTextSize(ICON_FA_INFO_CIRCLE).x, ImGui::CalcTextSize(ICON_FA_TIMES_CIRCLE).x) };
        ImGui::PushFont(theme::GetFonts().mono, 0.0f);
        const float iconX{ ImGui::GetCursorPosX() + ImGui::CalcTextSize("00:00:00").x + spacing };
        const float textX{ iconX + iconWidth + spacing };
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(shown.size()));
        while (clipper.Step()) {
            for (int i{ clipper.DisplayStart }; i != clipper.DisplayEnd; ++i) {
                DrawLine(*shown[static_cast<std::size_t>(i)], iconX, textX);
            }
        }
        clipper.End();
        ImGui::PopFont();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Copy", nullptr, false, !shown.empty())) {
                std::string text;
                for (const ConsoleLine* line : shown) {
                    text += (line->isFirst ? line->time + "  " : std::string(line->time.size() + 2, ' ')) + line->text + "\n";
                }
                ImGui::SetClipboardText(text.c_str());
            }
            if (ImGui::MenuItem("Clear")) {
                isCleared = true;
            }
            ImGui::EndPopup();
        }
        ImGui::EndChild();

        if (isCleared) {
            m_Lines.clear();
        }
    }

    bool ConsolePanel::OnLogEvent(EditorLogEvent& event) noexcept {
        const std::string time{ GetTimeOfDay() };
        std::string_view message{ event.message };
        if (message.ends_with('\n')) {
            message.remove_suffix(1);
        }
        bool isFirst{ true };
        while (true) {
            const std::size_t end{ message.find('\n') };
            m_Lines.push_back(ConsoleLine{ event.type, isFirst, time, ExpandTabs(message.substr(0, end)) });
            isFirst = false;
            if (end == std::string_view::npos) {
                break;
            }
            message.remove_prefix(end + 1);
        }
        while (m_Lines.size() > maxLines) {
            m_Lines.pop_front();
        }

        return true;
    }
} // namespace adh
