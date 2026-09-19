#pragma once

#include "watchflow/core/FileEvent.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace watchflow {

class FileLogger {
public:
    explicit FileLogger(std::filesystem::path file);

    void logEvent(const FileEvent& event);
    void logRuleResult(const std::string& ruleName, bool success);

private:
    std::ofstream output_;
};

} // namespace watchflow
