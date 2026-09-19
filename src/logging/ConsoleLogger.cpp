#include "watchflow/logging/ConsoleLogger.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace watchflow {

void ConsoleLogger::log(const FileEvent& event) const {
    const auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());

    std::cout << "[" << std::put_time(std::localtime(&now), "%H:%M:%S")
              << "] " << toString(event.type) << " "
              << event.path << "\n";
}

} // namespace watchflow
