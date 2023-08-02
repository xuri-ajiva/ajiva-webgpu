//
// Created by XuriAjiva on 31.07.2023.
//
#include "Logger.h"

#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>

#ifdef AJ_LOG_IMGUI

#include "imgui.h"
#include "magic_enum.hpp"
#include <list>

using namespace plog;

class MyFormatter {
public:
    static util::nstring header() // This method returns a header for a new file. In our case it is empty.
    {
        return util::nstring();
    }

    static util::nstring format(const Record &record) // This method returns a string from a record.
    {
        util::nostringstream ss;
        ss << record.getMessage() << "\n"; // Produce a simple string with a log message.

        return ss.str();
    }
};

template<class Formatter> // Typically a formatter is passed as a template parameter.
class ImGuiAppender : public IAppender // All appenders MUST inherit IAppender interface.
{
public:
    void write(const Record &record) PLOG_OVERRIDE // This is a method from IAppender that MUST be implemented.
    {
        m_messageList.push_back(record);
    }


    void Draw() {
        static ImVec4 Colors[7] = {
                {0.5f, 0.5f, 0.5f, 1.0f},//none = 0,
                {1.0f, 0.0f, 0.0f, 1.0f},//fatal = 1,
                {1.0f, 0.5f, 0.0f, 1.0f},//error = 2,
                {1.0f, 1.0f, 0.0f, 1.0f},//warning = 3,
                {1.0f, 1.0f, 1.0f, 1.0f},//info = 4,
                {0.7f, 1.0f, 1.0f, 1.0f},//debug = 5,
                {0.0f, 0.4f, 0.4f, 1.0f},//verbose = 6
        };

        // Options menu
        if (ImGui::BeginPopup("Options")) {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");

        ImGui::Checkbox("None", &SeverityFilter[0]);
        ImGui::SameLine();
        ImGui::Checkbox("Fatal", &SeverityFilter[1]);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &SeverityFilter[2]);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &SeverityFilter[3]);
        ImGui::SameLine();
        ImGui::Checkbox("Info", &SeverityFilter[4]);
        ImGui::SameLine();
        ImGui::Checkbox("Debug", &SeverityFilter[5]);
        ImGui::SameLine();
        ImGui::Checkbox("Verbose", &SeverityFilter[6]);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (clear)
                m_messageList.clear();
            if (copy)
                ImGui::LogToClipboard();


            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            if (ImGui::BeginTable("log", 6,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                                  ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                                  ImGuiTableFlags_ScrollX |
                                  ImGuiTableFlags_SizingStretchProp)) {

                ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 56);
                ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort,
                                        91);
                ImGui::TableSetupColumn("TID", ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableSetupColumn("Location",
                                        ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide,
                                        200);
                ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide,
                                        200);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableHeadersRow();

                for (auto &record: m_messageList) {
                    auto severity = record.getSeverity();
                    if (!SeverityFilter[severity]) continue;
                    tm t{};
                    util::localtime_s(&t, &record.getTime().time);

                    ImColor color = Colors[severity];

                    ImGui::TableNextRow();

                    if (severity > 0 && severity < Severity::info) {
                        ImGui::PushStyleColor(ImGuiCol_Text, /*black*/ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, color);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, color.Value);
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", severityToString(record.getSeverity()));
                    ImGui::TableNextColumn();
                    ImGui::Text("%02d:%02d:%02d.%03d ", t.tm_hour, t.tm_min, t.tm_sec, record.getTime().millitm);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", record.getTid());
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped("%s@%zu", record.getFunc(), record.getLine());
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped("%s:%zu", record.getFile(), record.getLine());
                    ImGui::TableNextColumn();
#ifdef _WIN32
                    std::wstring w_string(record.getMessage());
                    std::string string(w_string.begin(), w_string.end());
                    ImGui::TextWrapped("%s", string.c_str());
#else
                    ImGui::TextWrapped(record.getMessage());
#endif
                    ImGui::PopStyleColor();
                    ImGui::TableNextColumn();
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleVar();

            if (copy)
                ImGui::LogFinish();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }

private:
    std::list<Record> m_messageList;
    bool AutoScroll;  // Keep scrolling if already at the bottom.
    bool SeverityFilter[7] = {true, true, true, true, true, true, false};

};

static ImGuiAppender<plog::TxtFormatter> imgui_appender;

void Ajiva::Core::ShowAppLog(bool *p_open) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Log", p_open);
    imgui_appender.Draw();
    ImGui::End();
}

#else
inline static void Ajiva::Core::ShowAppLog(bool *p_open){}
#endif // AJ_LOG_IMGUI

class AjivaTxtFormatter {
public:
    static util::nstring header() {
        return util::nstring();
    }

    static util::nstring format(const Record &record) {
        tm t;
        util::localtime_s(&t, &record.getTime().time);

        util::nostringstream ss;
        ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-")
           << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
        ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":")
           << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0'))
           << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3)
           << static_cast<int> (record.getTime().millitm) << PLOG_NSTR(" ");
        ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity())
           << PLOG_NSTR(" ");
        ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
        ss << PLOG_NSTR("[") << record.getFile() << PLOG_NSTR(":") << record.getLine() << PLOG_NSTR("] ");
        ss << PLOG_NSTR("(") << record.getFunc() << PLOG_NSTR(") ");
        ss << record.getMessage() << PLOG_NSTR("\n");

        return ss.str();
    }
};

static plog::ColorConsoleAppender<AjivaTxtFormatter> consoleAppender;


void Ajiva::Core::SetupLogger() {
    plog::init(plog::verbose, &consoleAppender)
#ifdef AJ_LOG_IMGUI
            .addAppender(&imgui_appender)
#endif
            ;

    // Log severity levels are printed in different colors.
    PLOG_VERBOSE << "This is a VERBOSE message";
    PLOG_DEBUG << "This is a DEBUG message";
    PLOG_INFO << "This is an INFO message";
    PLOG_WARNING << "This is a WARNING message";
    PLOG_ERROR << "This is an ERROR message";
    PLOG_FATAL << "This is a FATAL message";
}
