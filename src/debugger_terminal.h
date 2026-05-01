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

inline void openDebuggerTerminal(Registry& registry, Entity player) {
    if (!registry.alive(player)) return;
    if (!registry.has<DebuggerTerminalComponent>(player)) {
        registry.assign<DebuggerTerminalComponent>(player);
    }
    registry.get<DebuggerTerminalComponent>(player).open = true;
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
