#include "watchflow/config/ConfigManager.hpp"
#include "watchflow/logging/ConsoleLogger.hpp"
#include "watchflow/watcher/InotifyFileWatcher.hpp"

#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>

namespace {
watchflow::IFileWatcher* watcher = nullptr;

void handleSignal(int) {
    if (watcher) watcher->stop();
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

        for (auto& rule : config.rules) {
            engine.addRule(std::move(rule));
        }

        watcher = &fileWatcher;
        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        std::cout << "WatchFlow started\n";
        std::cout << "Watching: " << config.watchDirectory << "\n";
        std::cout << "Rules: " << config.rules.size() << "\n";
        std::cout << "Press Ctrl+C to stop.\n\n";

        fileWatcher.start([&](const watchflow::FileEvent& event) {
            logger.log(event);
            engine.onEvent(event);
        });

        // Keep main thread alive. The watcher owns the event loop.
        while (true) {
            pause();
            if (std::cin.eof()) break;
        }

        fileWatcher.stop();
    } catch (const std::exception& ex) {
        std::cerr << "WatchFlow error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
