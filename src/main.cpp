#include "watchflow/config/ConfigManager.hpp"
#include "watchflow/logging/ConsoleLogger.hpp"
#include "watchflow/logging/FileLogger.hpp"
#include "watchflow/rule/RuleEngine.hpp"
#include "watchflow/watcher/InotifyFileWatcher.hpp"

#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <unistd.h>

namespace {
volatile std::sig_atomic_t keepRunning = 1;

void handleSignal(int) {
    keepRunning = 0;
}
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: watchflow <config-file>\n";
        return 1;
    }

    try {
        watchflow::ConfigManager configManager;
        auto config = configManager.load(argv[1]);

        if (!std::filesystem::is_directory(config.watchDirectory)) {
            std::cerr << "Watch directory does not exist: "
                      << config.watchDirectory << "\n";
            return 1;
        }

        watchflow::InotifyFileWatcher fileWatcher(config.watchDirectory);
        watchflow::RuleEngine engine;
        watchflow::ConsoleLogger logger;
        std::optional<watchflow::FileLogger> fileLogger;

        if (!config.logFile.empty()) {
            fileLogger.emplace(config.logFile);
        }

        for (auto& rule : config.rules) {
            engine.addRule(std::move(rule));
        }

        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        std::cout << "WatchFlow started\n";
        std::cout << "Watching: " << config.watchDirectory << "\n";
        std::cout << "Rules: " << config.rules.size() << "\n";
        if (!config.logFile.empty()) {
            std::cout << "Log file: " << config.logFile << "\n";
        }
        std::cout << "Press Ctrl+C to stop.\n\n";

        fileWatcher.start([&](const watchflow::FileEvent& event) {
            logger.log(event);
            if (fileLogger) {
                fileLogger->logEvent(event);
            }

            const auto results = engine.onEvent(event);
            if (fileLogger) {
                for (const auto& [ruleName, success] : results) {
                    fileLogger->logRuleResult(ruleName, success);
                }
            }
        });

        // Keep main thread alive. The watcher owns the event loop.
        while (keepRunning) {
            pause();
        }

        fileWatcher.stop();
    } catch (const std::exception& ex) {
        std::cerr << "WatchFlow error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
