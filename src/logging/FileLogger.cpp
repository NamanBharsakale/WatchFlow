#include "watchflow/logging/FileLogger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <stdexcept>

namespace watchflow {

namespace {
void writeTimestamp(std::ofstream& output) {
    const auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    output << "[" << std::put_time(std::localtime(&now), "%F %T") << "] ";
}
}

FileLogger::FileLogger(std::filesystem::path file) {
    std::error_code ec;
    if (!file.parent_path().empty()) {
        std::filesystem::create_directories(file.parent_path(), ec);
        if (ec) {
            throw std::runtime_error("Cannot create log directory: " + file.parent_path().string());
        }
    }

    output_.open(file, std::ios::app);
    if (!output_) {
        throw std::runtime_error("Cannot open log file: " + file.string());
    }
}

void FileLogger::logEvent(const FileEvent& event) {
    writeTimestamp(output_);
    output_ << toString(event.type) << " " << event.path << "\n";
    output_.flush();
}

void FileLogger::logRuleResult(const std::string& ruleName, bool success) {
    writeTimestamp(output_);
    output_ << "RULE " << ruleName << " " << (success ? "SUCCESS" : "FAILED") << "\n";
    output_.flush();
}

} // namespace watchflow
