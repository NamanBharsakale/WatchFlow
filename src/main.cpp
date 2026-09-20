#include "watchflow/config/ConfigManager.hpp"
#include "watchflow/logging/ConsoleLogger.hpp"
#include "watchflow/logging/FileLogger.hpp"
#include "watchflow/rule/RuleEngine.hpp"
#include "watchflow/watcher/InotifyFileWatcher.hpp"

#include <csignal>
#include <filesystem>
#include <iostream>
#include <optional>
#include <unistd.h>

using namespace std;
using namespace watchflow;
namespace {
    volatile sig_atomic_t keepRunning = 1;

    void handleSignal(int) {
        keepRunning = 0;
    }
}

int main(int argc, char* argv[]) {

    // Check command-line arguments
    if (argc != 2) {
        cerr << "Usage: watchflow <config-file>\n";
        return 1;
    }

    try {

        // 1. Load configuration
        ConfigManager configManager;
        auto config = configManager.load(argv[1]);

        // 2. Check watch directory
        if (!filesystem::is_directory(config.watchDirectory)) {
            cerr << "Watch directory does not exist: "
                 << config.watchDirectory << "\n";
            return 1;
        }

        // 3. Create WatchFlow components
        InotifyFileWatcher fileWatcher(config.watchDirectory);
        RuleEngine engine;
        ConsoleLogger logger;

        optional<FileLogger> fileLogger;

        // Create file logger only if configured
        if (!config.logFile.empty()) {
            fileLogger.emplace(config.logFile);
        }

        // 4. Add rules to RuleEngine
        for (auto& rule : config.rules) {
            engine.addRule(move(rule));
        }

        // 5. Handle Ctrl+C and termination
        signal(SIGINT, handleSignal);
        signal(SIGTERM, handleSignal);

        // 6. Display startup information
        cout << "WatchFlow started\n";
        cout << "Watching: " << config.watchDirectory << "\n";
        cout << "Rules: " << config.rules.size() << "\n";

        if (!config.logFile.empty()) {
            cout << "Log file: " << config.logFile << "\n";
        }

        cout << "Press Ctrl+C to stop.\n\n";

        // 7. Start watching files
        fileWatcher.start([&](const FileEvent& event) {

            // Print event on console
            logger.log(event);

            // Write event to file if enabled
            if (fileLogger) {
                fileLogger->logEvent(event);
            }

            // Process event using rules
            auto results = engine.onEvent(event);

            // Log rule results
            if (fileLogger) {
                for (const auto& [ruleName, success] : results) {
                    fileLogger->logRuleResult(ruleName, success);
                }
            }
        });

        // 8. Keep program running
        while (keepRunning) {
            pause();
        }

        // 9. Stop watcher
        fileWatcher.stop();

    }
    catch (const exception& ex) {
        cerr << "WatchFlow error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}