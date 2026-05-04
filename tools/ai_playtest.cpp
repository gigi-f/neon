#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "ai_playtest.h"

namespace {

const char* kDefaultStatePath = ".neon_ai_playtest_state.txt";

void printUsage() {
    std::cout
        << "Usage:\n"
        << "  neon_ai_playtest snapshot [--state PATH]\n"
        << "  neon_ai_playtest reset [default|suspicion|market|shelter] [--state PATH]\n"
        << "  neon_ai_playtest key W|A|S|D|E|F|T|SPACE|G [--state PATH]\n"
        << "  neon_ai_playtest step [N] [--state PATH]\n"
        << "  neon_ai_playtest warp TARGET [--state PATH]\n"
        << "  neon_ai_playtest play [default|suspicion|market|shelter] [--state PATH] [--transcript PATH]\n"
        << "  neon_ai_playtest run [default|suspicion|market|shelter|all] [--state PATH] [--pause-ms N]\n";
}

std::string statePathFromArgs(int argc, char** argv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--state") {
            return argv[i + 1];
        }
    }
    return kDefaultStatePath;
}

std::string positionalArg(int argc, char** argv, int index, const std::string& fallback = "") {
    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--state" || arg == "--pause-ms" || arg == "--transcript") {
            ++i;
            continue;
        }
        if (positional == index) {
            return arg;
        }
        ++positional;
    }
    return fallback;
}

std::string stringOptionFromArgs(int argc,
                                 char** argv,
                                 const std::string& option,
                                 const std::string& fallback = "") {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == option) {
            return argv[i + 1];
        }
    }
    return fallback;
}

int intOptionFromArgs(int argc, char** argv, const std::string& option, int fallback) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == option) {
            return std::max(0, std::atoi(argv[i + 1]));
        }
    }
    return fallback;
}

struct TerminalPlaytestStep {
    std::string label;
    std::string command;
    std::string argument;
};

void printTerminalPlaytestSnapshot(const std::string& heading,
                                   const std::string& result,
                                   AiPlaytestSession& session,
                                   int pause_ms) {
    std::cout << "\n--- " << heading << " ---\n";
    if (!result.empty()) {
        std::cout << result << "\n";
    }
    std::cout << aiPlaytestSnapshot(session);
    std::cout.flush();

    if (pause_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pause_ms));
    }
}

bool applyTerminalPlaytestStep(AiPlaytestSession& session,
                               const TerminalPlaytestStep& step,
                               std::string* result) {
    if (step.command == "key") {
        return applyAiPlaytestKey(session, step.argument, result);
    }
    if (step.command == "warp") {
        return warpAiPlaytestPlayer(session, step.argument, result);
    }
    if (step.command == "step") {
        const int count = std::max(1, std::atoi(step.argument.c_str()));
        for (int i = 0; i < count; ++i) {
            advanceAiPlaytestSimulation(session.registry, kAiPlaytestStepDt);
        }
        if (result) {
            *result = "STEP " + std::to_string(count) + " OK";
        }
        return true;
    }
    if (result) {
        *result = "UNKNOWN PLAYTEST STEP: " + step.command;
    }
    return false;
}

bool runTerminalPlaytestScenario(const std::string& name,
                                 AiPlaytestScenario scenario,
                                 const std::vector<TerminalPlaytestStep>& steps,
                                 const std::string& state_path,
                                 int pause_ms) {
    AiPlaytestSession session;
    if (!buildAiPlaytestSession(session, scenario)) {
        std::cerr << "Failed to build AI playtest session.\n";
        return false;
    }

    std::cout << "\n==============================\n";
    std::cout << "NEON TERMINAL PLAYTEST: " << name << "\n";
    std::cout << "==============================\n";
    printTerminalPlaytestSnapshot("0 RESET " + name, "RESET " + name + " OK", session, pause_ms);

    for (std::size_t i = 0; i < steps.size(); ++i) {
        std::string result;
        if (!applyTerminalPlaytestStep(session, steps[i], &result)) {
            std::cerr << result << "\n";
            return false;
        }
        printTerminalPlaytestSnapshot(
            std::to_string(i + 1) + "/" + std::to_string(steps.size()) + " " + steps[i].label,
            result,
            session,
            pause_ms);
    }

    if (!saveAiPlaytestSession(session, state_path)) {
        std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
        return false;
    }

    std::cout << "\n=== FINAL OUTPUT: " << name << " ===\n";
    std::cout << aiPlaytestSnapshot(session);
    return true;
}

bool runTerminalPlaytest(const std::string& raw_scenario,
                         const std::string& state_path,
                         int pause_ms) {
    const std::string scenario = normalizeAiPlaytestKey(raw_scenario.empty() ? "all" : raw_scenario);
    const std::vector<TerminalPlaytestStep> default_steps = {
        {"MOVE NORTH", "key", "W"},
        {"MOVE EAST", "key", "D"},
        {"WARP TO TRANSIT", "warp", "STATION"},
        {"BOARD TRANSIT", "key", "E"},
        {"LOOK OUT WINDOW TO NEXT DISTRICT", "key", "E"},
        {"WARP TO LOCAL SIGNPOST", "warp", "SIGNPOST"},
        {"SCAN LOCAL SIGNPOST", "key", "SPACE"},
        {"WARP TO WORKER", "warp", "WORKER"},
        {"SCAN WORKER", "key", "SPACE"},
        {"LET SIMULATION ADVANCE", "step", "3"},
    };
    const std::vector<TerminalPlaytestStep> suspicion_steps = {
        {"SPOOF WAGE RECORD", "key", "G"},
        {"WARP TO WORKER", "warp", "WORKER"},
        {"RESTORE WAGE RECORD", "key", "G"},
        {"WARP TO CLINIC", "warp", "CLINIC"},
        {"TRY RECORDS BOUNDARY WITHOUT CLEARANCE", "key", "E"},
        {"SPOOF CLINIC CLEARANCE", "key", "G"},
        {"CROSS RECORDS BOUNDARY WITH CLEARANCE", "key", "E"},
        {"RESTORE CLINIC CLEARANCE", "key", "G"},
        {"EXIT CLINIC", "key", "E"},
        {"TRY RESTORED RECORDS BOUNDARY", "key", "E"},
    };
    const std::vector<TerminalPlaytestStep> market_steps = {
        {"WARP TO MARKET", "warp", "MARKET"},
        {"INSPECT MARKET", "key", "SPACE"},
        {"EXCHANGE AT MARKET EMPTY HANDS", "key", "E"},
        {"INSPECT MARKET LEDGER AFTER EXCHANGE", "key", "SPACE"},
        {"STEP SIMULATION", "step", "2"},
    };
    const std::vector<TerminalPlaytestStep> shelter_steps = {
        {"WARP TO SHELTER LISTING", "warp", "LISTING"},
        {"INSPECT SHELTER LISTING", "key", "SPACE"},
        {"MARK SHELTER INTEREST", "key", "E"},
        {"INSPECT MARKED LISTING", "key", "SPACE"},
        {"CLEAR SHELTER INTEREST", "key", "E"},
        {"INSPECT CLEARED LISTING", "key", "SPACE"},
    };

    bool ok = true;
    if (scenario == "DEFAULT" || scenario == "ALL") {
        ok = runTerminalPlaytestScenario("DEFAULT",
                                         AiPlaytestScenario::DEFAULT,
                                         default_steps,
                                         state_path,
                                         pause_ms) && ok;
    }
    if (scenario == "SUSPICION" || scenario == "ALL") {
        ok = runTerminalPlaytestScenario("SUSPICION",
                                         AiPlaytestScenario::SUSPICION,
                                         suspicion_steps,
                                         state_path,
                                         pause_ms) && ok;
    }
    if (scenario == "MARKET" || scenario == "ALL") {
        ok = runTerminalPlaytestScenario("MARKET",
                                         AiPlaytestScenario::MARKET,
                                         market_steps,
                                         state_path,
                                         pause_ms) && ok;
    }
    if (scenario == "SHELTER" || scenario == "ALL") {
        ok = runTerminalPlaytestScenario("SHELTER",
                                         AiPlaytestScenario::SHELTER,
                                         shelter_steps,
                                         state_path,
                                         pause_ms) && ok;
    }
    if (scenario != "DEFAULT" && scenario != "SUSPICION" && scenario != "MARKET" &&
        scenario != "SHELTER" && scenario != "ALL") {
        std::cerr << "Unknown run scenario: " << raw_scenario << "\n";
        return false;
    }

    std::cout << "\nPLAYTEST " << scenario << (ok ? " PASSED" : " FAILED") << "\n";
    return ok;
}

bool isDirectPlaytestKey(const std::string& command) {
    return command == "W" || command == "A" || command == "S" || command == "D" ||
           command == "E" || command == "F" || command == "T" || command == "G" ||
           command == "SPACE" || command == "WAIT" || command == ".";
}

void printInteractiveHelp(std::ostream& out) {
    out << "Commands: W A S D E F T G SPACE WAIT | key KEY | step [N] | warp TARGET | "
        << "reset [default|suspicion|market|shelter] | snapshot | help | quit\n";
}

bool runInteractivePlaytest(const std::string& raw_scenario,
                            const std::string& state_path,
                            const std::string& transcript_path) {
    std::ofstream transcript;
    if (!transcript_path.empty()) {
        transcript.open(transcript_path, std::ios::out | std::ios::trunc);
        if (!transcript) {
            std::cerr << "Failed to open transcript: " << transcript_path << "\n";
            return false;
        }
    }

    auto emit = [&](const std::string& text) {
        std::cout << text;
        if (transcript) {
            transcript << text;
        }
    };

    AiPlaytestSession session;
    std::string result;
    const bool requested_reset = !raw_scenario.empty();
    if (requested_reset) {
        AiPlaytestScenario scenario = AiPlaytestScenario::DEFAULT;
        if (!parseAiPlaytestScenario(raw_scenario, scenario)) {
            std::cerr << "Unknown play scenario: " << raw_scenario << "\n";
            return false;
        }
        if (!buildAiPlaytestSession(session, scenario)) {
            std::cerr << "Failed to build AI playtest session.\n";
            return false;
        }
        result = "RESET " + normalizeAiPlaytestKey(raw_scenario) + " OK";
    } else if (aiPlaytestFileExists(state_path)) {
        std::string error;
        if (!loadAiPlaytestSession(session, state_path, &error)) {
            std::cerr << error << "\n";
            return false;
        }
        result = "LOADED STATE " + state_path;
    } else {
        if (!buildAiPlaytestSession(session)) {
            std::cerr << "Failed to build AI playtest session.\n";
            return false;
        }
        result = "RESET DEFAULT OK";
    }

    if (!saveAiPlaytestSession(session, state_path)) {
        std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
        return false;
    }

    emit("=== NEON INTERACTIVE PLAYTEST ===\n");
    printInteractiveHelp(std::cout);
    if (transcript) {
        printInteractiveHelp(transcript);
    }
    emit(result + "\n");
    emit(aiPlaytestSnapshot(session));

    std::string line;
    while (true) {
        std::cout << "\nai> ";
        std::cout.flush();
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (transcript) {
            transcript << "\nai> " << line << "\n";
        }
        if (line.empty()) {
            continue;
        }

        std::istringstream input(line);
        std::string verb;
        input >> verb;
        const std::string command = normalizeAiPlaytestKey(verb);
        bool should_save = false;
        bool show_snapshot = true;
        result.clear();

        if (command == "QUIT" || command == "EXIT") {
            result = "PLAYTEST SESSION ENDED";
            break;
        }
        if (command == "HELP" || command == "?") {
            printInteractiveHelp(std::cout);
            if (transcript) {
                printInteractiveHelp(transcript);
            }
            continue;
        }
        if (command == "SNAPSHOT") {
            result = "SNAPSHOT OK";
        } else if (command == "RESET") {
            std::string scenario_arg;
            input >> scenario_arg;
            if (scenario_arg.empty()) {
                scenario_arg = "default";
            }
            AiPlaytestScenario scenario = AiPlaytestScenario::DEFAULT;
            if (!parseAiPlaytestScenario(scenario_arg, scenario) ||
                !buildAiPlaytestSession(session, scenario)) {
                emit("UNKNOWN RESET SCENARIO: " + scenario_arg + "\n");
                continue;
            }
            result = "RESET " + normalizeAiPlaytestKey(scenario_arg) + " OK";
            should_save = true;
        } else if (command == "KEY") {
            std::string key;
            input >> key;
            if (key.empty() || !applyAiPlaytestKey(session, key, &result)) {
                emit((result.empty() ? "MISSING KEY" : result) + "\n");
                continue;
            }
            should_save = true;
        } else if (isDirectPlaytestKey(command)) {
            if (!applyAiPlaytestKey(session, command, &result)) {
                emit(result + "\n");
                continue;
            }
            should_save = true;
        } else if (command == "STEP") {
            int steps = 1;
            std::string steps_arg;
            input >> steps_arg;
            if (!steps_arg.empty()) {
                steps = std::max(1, std::atoi(steps_arg.c_str()));
            }
            for (int i = 0; i < steps; ++i) {
                advanceAiPlaytestSimulation(session.registry, kAiPlaytestStepDt);
            }
            result = "STEP " + std::to_string(steps) + " OK";
            should_save = true;
        } else if (command == "WARP") {
            std::string target;
            input >> target;
            if (target.empty() || !warpAiPlaytestPlayer(session, target, &result)) {
                emit((result.empty() ? "MISSING WARP TARGET" : result) + "\n");
                continue;
            }
            should_save = true;
        } else {
            emit("UNKNOWN COMMAND: " + verb + "\n");
            show_snapshot = false;
        }

        if (should_save && !saveAiPlaytestSession(session, state_path)) {
            std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
            return false;
        }
        if (show_snapshot) {
            emit(result + "\n");
            emit(aiPlaytestSnapshot(session));
        }
    }

    saveAiPlaytestSession(session, state_path);
    emit("\n=== FINAL OUTPUT ===\n");
    if (!result.empty()) {
        emit(result + "\n");
    }
    emit(aiPlaytestSnapshot(session));
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc <= 1) {
        printUsage();
        return 1;
    }

    const std::string command = normalizeAiPlaytestKey(positionalArg(argc, argv, 0));
    const std::string state_path = statePathFromArgs(argc, argv);
    AiPlaytestSession session;
    std::string error;

    if (command == "RUN") {
        const std::string scenario_arg = positionalArg(argc, argv, 1, "all");
        const int pause_ms = intOptionFromArgs(argc, argv, "--pause-ms", 0);
        return runTerminalPlaytest(scenario_arg, state_path, pause_ms) ? 0 : 2;
    }

    if (command == "PLAY") {
        const std::string scenario_arg = positionalArg(argc, argv, 1);
        const std::string transcript_path =
            stringOptionFromArgs(argc, argv, "--transcript");
        return runInteractivePlaytest(scenario_arg, state_path, transcript_path) ? 0 : 2;
    }

    if (command == "RESET") {
        AiPlaytestScenario scenario = AiPlaytestScenario::DEFAULT;
        const std::string scenario_arg = positionalArg(argc, argv, 1, "default");
        if (!parseAiPlaytestScenario(scenario_arg, scenario)) {
            std::cerr << "Unknown reset scenario: " << scenario_arg << "\n";
            return 2;
        }
        if (!buildAiPlaytestSession(session, scenario)) {
            std::cerr << "Failed to build AI playtest session.\n";
            return 2;
        }
        if (!saveAiPlaytestSession(session, state_path)) {
            std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
            return 2;
        }
        std::cout << "RESET " << normalizeAiPlaytestKey(scenario_arg) << " OK\n";
        std::cout << aiPlaytestSnapshot(session);
        return 0;
    }

    if (!loadAiPlaytestSession(session, state_path, &error)) {
        std::cerr << error << "\n";
        return 2;
    }

    if (command == "SNAPSHOT") {
        std::cout << aiPlaytestSnapshot(session);
        return 0;
    }

    if (command == "KEY") {
        const std::string key = positionalArg(argc, argv, 1);
        std::string result;
        if (key.empty() || !applyAiPlaytestKey(session, key, &result)) {
            std::cerr << (result.empty() ? "Missing key." : result) << "\n";
            return 2;
        }
        if (!saveAiPlaytestSession(session, state_path)) {
            std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
            return 2;
        }
        std::cout << result << "\n";
        std::cout << aiPlaytestSnapshot(session);
        return 0;
    }

    if (command == "STEP") {
        int steps = 1;
        const std::string steps_arg = positionalArg(argc, argv, 1);
        if (!steps_arg.empty()) {
            steps = std::max(1, std::atoi(steps_arg.c_str()));
        }
        for (int i = 0; i < steps; ++i) {
            advanceAiPlaytestSimulation(session.registry, kAiPlaytestStepDt);
        }
        if (!saveAiPlaytestSession(session, state_path)) {
            std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
            return 2;
        }
        std::cout << "STEP " << steps << " OK\n";
        std::cout << aiPlaytestSnapshot(session);
        return 0;
    }

    if (command == "WARP") {
        const std::string target = positionalArg(argc, argv, 1);
        std::string result;
        if (target.empty() || !warpAiPlaytestPlayer(session, target, &result)) {
            std::cerr << (result.empty() ? "Missing warp target." : result) << "\n";
            return 2;
        }
        if (!saveAiPlaytestSession(session, state_path)) {
            std::cerr << "Failed to save AI playtest state: " << state_path << "\n";
            return 2;
        }
        std::cout << result << "\n";
        std::cout << aiPlaytestSnapshot(session);
        return 0;
    }

    printUsage();
    return 1;
}
