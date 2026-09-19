#pragma once

#include "watchflow/rule/Rule.hpp"
#include <filesystem>
#include <vector>

namespace watchflow {

struct Config {
    std::filesystem::path watchDirectory;
    std::vector<Rule> rules;
};

class ConfigManager {
public:
    Config load(const std::filesystem::path& file) const;
};

} // namespace watchflow
