#pragma once

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "world_builder.h"

struct DebuggerTerminalContent {
    bool open = false;
    std::string title;
    std::vector<std::string> lines;
};

struct DebuggerTerminalRect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

enum class DebuggerTerminalHitRegion {
    NONE,
    WINDOW,
    TITLE_BAR,
    MINIMIZE,
    CLOSE,
    RESTORE_ICON
};

constexpr int DEBUGGER_TERMINAL_TITLE_BAR_H = 22;
constexpr int DEBUGGER_TERMINAL_CONTROL_SIZE = 14;
constexpr int DEBUGGER_TERMINAL_CONTROL_GAP = 4;
constexpr int DEBUGGER_TERMINAL_ICON_W = 28;
constexpr int DEBUGGER_TERMINAL_ICON_H = 24;
constexpr int DEBUGGER_TERMINAL_ICON_MARGIN = 8;

inline bool debuggerTerminalPointInRect(int px,
                                        int py,
                                        const DebuggerTerminalRect& rect) {
    return px >= rect.x && px < rect.x + rect.w &&
           py >= rect.y && py < rect.y + rect.h;
}

inline DebuggerTerminalRect debuggerTerminalWindowRect(
        const DebuggerTerminalComponent& terminal,
        int screen_w,
        int screen_h) {
    const int safe_screen_w = std::max(1, screen_w);
    const int safe_screen_h = std::max(1, screen_h);
    const int width = std::min(terminal.width, safe_screen_w);
    const int height = std::min(terminal.height, safe_screen_h);
    return DebuggerTerminalRect{
        std::clamp(terminal.x, 0, std::max(0, safe_screen_w - width)),
        std::clamp(terminal.y, 0, std::max(0, safe_screen_h - height)),
        width,
        height
    };
}

inline DebuggerTerminalRect debuggerTerminalTitleBarRect(
        const DebuggerTerminalComponent& terminal,
        int screen_w,
        int screen_h) {
    const DebuggerTerminalRect window =
        debuggerTerminalWindowRect(terminal, screen_w, screen_h);
    return DebuggerTerminalRect{
        window.x,
        window.y,
        window.w,
        std::min(DEBUGGER_TERMINAL_TITLE_BAR_H, window.h)
    };
}

inline DebuggerTerminalRect debuggerTerminalCloseButtonRect(
        const DebuggerTerminalComponent& terminal,
        int screen_w,
        int screen_h) {
    const DebuggerTerminalRect title_bar =
        debuggerTerminalTitleBarRect(terminal, screen_w, screen_h);
    return DebuggerTerminalRect{
        title_bar.x + title_bar.w - DEBUGGER_TERMINAL_CONTROL_SIZE -
            DEBUGGER_TERMINAL_CONTROL_GAP,
        title_bar.y + DEBUGGER_TERMINAL_CONTROL_GAP,
        DEBUGGER_TERMINAL_CONTROL_SIZE,
        DEBUGGER_TERMINAL_CONTROL_SIZE
    };
}

inline DebuggerTerminalRect debuggerTerminalMinimizeButtonRect(
        const DebuggerTerminalComponent& terminal,
        int screen_w,
        int screen_h) {
    const DebuggerTerminalRect close_button =
        debuggerTerminalCloseButtonRect(terminal, screen_w, screen_h);
    return DebuggerTerminalRect{
        close_button.x - DEBUGGER_TERMINAL_CONTROL_SIZE -
            DEBUGGER_TERMINAL_CONTROL_GAP,
        close_button.y,
        DEBUGGER_TERMINAL_CONTROL_SIZE,
        DEBUGGER_TERMINAL_CONTROL_SIZE
    };
}

inline DebuggerTerminalRect debuggerTerminalRestoreIconRect(int screen_w,
                                                            int screen_h) {
    const int safe_screen_w = std::max(1, screen_w);
    const int safe_screen_h = std::max(1, screen_h);
    return DebuggerTerminalRect{
        DEBUGGER_TERMINAL_ICON_MARGIN,
        std::max(0, safe_screen_h - DEBUGGER_TERMINAL_ICON_H -
                     DEBUGGER_TERMINAL_ICON_MARGIN),
        std::min(DEBUGGER_TERMINAL_ICON_W, safe_screen_w),
        std::min(DEBUGGER_TERMINAL_ICON_H, safe_screen_h)
    };
}

inline void constrainDebuggerTerminalToViewport(DebuggerTerminalComponent& terminal,
                                                int screen_w,
                                                int screen_h) {
    const DebuggerTerminalRect rect =
        debuggerTerminalWindowRect(terminal, screen_w, screen_h);
    terminal.x = rect.x;
    terminal.y = rect.y;
}

inline DebuggerTerminalHitRegion debuggerTerminalHitRegion(
        const DebuggerTerminalComponent& terminal,
        int screen_w,
        int screen_h,
        int mouse_x,
        int mouse_y) {
    if (!terminal.open) {
        return DebuggerTerminalHitRegion::NONE;
    }
    if (terminal.minimized) {
        return debuggerTerminalPointInRect(
                   mouse_x,
                   mouse_y,
                   debuggerTerminalRestoreIconRect(screen_w, screen_h)) ?
                   DebuggerTerminalHitRegion::RESTORE_ICON :
                   DebuggerTerminalHitRegion::NONE;
    }

    if (debuggerTerminalPointInRect(
            mouse_x,
            mouse_y,
            debuggerTerminalCloseButtonRect(terminal, screen_w, screen_h))) {
        return DebuggerTerminalHitRegion::CLOSE;
    }
    if (debuggerTerminalPointInRect(
            mouse_x,
            mouse_y,
            debuggerTerminalMinimizeButtonRect(terminal, screen_w, screen_h))) {
        return DebuggerTerminalHitRegion::MINIMIZE;
    }
    if (debuggerTerminalPointInRect(
            mouse_x,
            mouse_y,
            debuggerTerminalTitleBarRect(terminal, screen_w, screen_h))) {
        return DebuggerTerminalHitRegion::TITLE_BAR;
    }
    if (debuggerTerminalPointInRect(
            mouse_x,
            mouse_y,
            debuggerTerminalWindowRect(terminal, screen_w, screen_h))) {
        return DebuggerTerminalHitRegion::WINDOW;
    }
    return DebuggerTerminalHitRegion::NONE;
}

inline void openDebuggerTerminal(Registry& registry, Entity player) {
    if (!registry.alive(player)) return;
    if (!registry.has<DebuggerTerminalComponent>(player)) {
        registry.assign<DebuggerTerminalComponent>(player);
    }
    auto& terminal = registry.get<DebuggerTerminalComponent>(player);
    terminal.open = true;
    terminal.minimized = false;
    terminal.dragging = false;
}

inline void closeDebuggerTerminal(DebuggerTerminalComponent& terminal) {
    terminal.open = false;
    terminal.minimized = false;
    terminal.dragging = false;
}

inline void minimizeDebuggerTerminal(DebuggerTerminalComponent& terminal) {
    if (!terminal.open) return;
    terminal.minimized = true;
    terminal.dragging = false;
}

inline void restoreDebuggerTerminal(DebuggerTerminalComponent& terminal) {
    if (!terminal.open) return;
    terminal.minimized = false;
    terminal.dragging = false;
}

inline bool beginDebuggerTerminalDrag(DebuggerTerminalComponent& terminal,
                                      int screen_w,
                                      int screen_h,
                                      int mouse_x,
                                      int mouse_y) {
    if (debuggerTerminalHitRegion(terminal, screen_w, screen_h, mouse_x, mouse_y) !=
        DebuggerTerminalHitRegion::TITLE_BAR) {
        return false;
    }

    const DebuggerTerminalRect rect =
        debuggerTerminalWindowRect(terminal, screen_w, screen_h);
    terminal.x = rect.x;
    terminal.y = rect.y;
    terminal.dragging = true;
    terminal.drag_offset_x = mouse_x - rect.x;
    terminal.drag_offset_y = mouse_y - rect.y;
    return true;
}

inline void dragDebuggerTerminalTo(DebuggerTerminalComponent& terminal,
                                   int screen_w,
                                   int screen_h,
                                   int mouse_x,
                                   int mouse_y) {
    if (!terminal.dragging) return;
    terminal.x = mouse_x - terminal.drag_offset_x;
    terminal.y = mouse_y - terminal.drag_offset_y;
    constrainDebuggerTerminalToViewport(terminal, screen_w, screen_h);
}

inline void endDebuggerTerminalDrag(DebuggerTerminalComponent& terminal) {
    terminal.dragging = false;
}

inline std::string trimDebuggerTerminalToken(const std::string& token) {
    size_t begin = 0;
    while (begin < token.size() &&
           std::isspace(static_cast<unsigned char>(token[begin]))) {
        ++begin;
    }

    size_t end = token.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(token[end - 1]))) {
        --end;
    }
    return token.substr(begin, end - begin);
}

inline std::vector<std::string> splitDebuggerTerminalReadout(const std::string& readout) {
    std::vector<std::string> lines;
    std::stringstream stream(readout);
    std::string token;
    while (std::getline(stream, token, ';')) {
        token = trimDebuggerTerminalToken(token);
        if (!token.empty()) {
            lines.push_back(token);
        }
    }
    if (lines.empty() && !readout.empty()) {
        lines.push_back(readout);
    }
    return lines;
}

inline std::vector<std::string> wrapDebuggerTerminalLine(const std::string& line,
                                                         size_t max_chars) {
    if (max_chars == 0 || line.size() <= max_chars) {
        return {line};
    }

    std::vector<std::string> wrapped;
    size_t start = 0;
    while (start < line.size()) {
        const size_t remaining = line.size() - start;
        if (remaining <= max_chars) {
            wrapped.push_back(line.substr(start));
            break;
        }

        size_t end = start + max_chars;
        size_t break_at = line.rfind(' ', end);
        if (break_at == std::string::npos || break_at <= start) {
            break_at = end;
        }
        wrapped.push_back(line.substr(start, break_at - start));
        start = break_at;
        while (start < line.size() && line[start] == ' ') {
            ++start;
        }
    }
    return wrapped;
}

inline DebuggerTerminalContent debuggerTerminalContent(Registry& registry,
                                                       Entity player,
                                                       float range_wu) {
    DebuggerTerminalContent content;
    if (!registry.alive(player) ||
        !registry.has<DebuggerTerminalComponent>(player) ||
        !registry.get<DebuggerTerminalComponent>(player).open) {
        return content;
    }

    content.open = true;
    content.title = "MOTHER'S DEBUGGER // TERMINAL";
    content.lines.push_back(inheritedGadgetReadout(registry, player));
    content.lines.push_back(inheritedGadgetPromptReadout(registry, player, range_wu));
    content.lines.push_back(inheritedGadgetSpoofPromptReadout(registry, player, range_wu));

    if (!registry.has<InheritedGadgetComponent>(player)) {
        content.lines.push_back("DEBUGGER RESULT: UNAVAILABLE");
        return content;
    }

    const auto& gadget = registry.get<InheritedGadgetComponent>(player);
    if (gadget.last_result_target_entity != MAX_ENTITIES) {
        content.lines.push_back(std::string("TARGET: ") +
                                inheritedGadgetTargetLabel(gadget.last_result_target_type) +
                                " entity=" +
                                std::to_string(gadget.last_result_target_entity));
    } else if (!gadget.last_result.empty()) {
        content.lines.push_back("TARGET: NO TARGET");
    } else {
        content.lines.push_back("TARGET: IDLE");
    }

    const std::vector<std::string> readout_lines =
        splitDebuggerTerminalReadout(inheritedGadgetResultReadout(registry, player));
    content.lines.insert(content.lines.end(), readout_lines.begin(), readout_lines.end());
    return content;
}
