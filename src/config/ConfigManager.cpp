#include "watchflow/config/ConfigManager.hpp"

#include "watchflow/action/CompressAction.hpp"
#include "watchflow/action/CopyAction.hpp"
#include "watchflow/action/ExecuteAction.hpp"
#include "watchflow/action/NotificationAction.hpp"
#include "watchflow/matcher/FileSizeMatcher.hpp"
#include "watchflow/matcher/RegexMatcher.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace watchflow {

namespace {

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, delimiter)) parts.push_back(item);
    return parts;
}

EventType parseEvent(const std::string& value) {
    if (value == "CREATED") return EventType::Created;
    if (value == "MODIFIED") return EventType::Modified;
    if (value == "DELETED") return EventType::Deleted;
    if (value == "MOVED") return EventType::Moved;
    throw std::runtime_error("Unknown event: " + value);
}

std::unique_ptr<IMatcher> makeMatcher(const std::string& type, const std::string& value) {
    if (type == "REGEX") {
        return std::make_unique<RegexMatcher>(value);
    }

    if (type == "SIZE_GT") {
        return std::make_unique<FileSizeMatcher>(
            std::stoull(value), SizeComparison::GreaterThan);
    }

    if (type == "SIZE_LT") {
        return std::make_unique<FileSizeMatcher>(
            std::stoull(value), SizeComparison::LessThan);
    }

    if (type == "SIZE_EQ") {
        return std::make_unique<FileSizeMatcher>(
            std::stoull(value), SizeComparison::Equal);
    }

    throw std::runtime_error("Unknown matcher: " + type);
}

std::unique_ptr<IAction> makeAction(const std::string& type, const std::string& value) {
    if (type == "COPY") {
        return std::make_unique<CopyAction>(value);
    }

    if (type == "EXECUTE") {
        return std::make_unique<ExecuteAction>(value);
    }

    if (type == "COMPRESS") {
        return std::make_unique<CompressAction>(value.empty() ? ".gz" : value);
    }

    if (type == "NOTIFY") {
        return std::make_unique<NotificationAction>(value);
    }

    throw std::runtime_error("Unknown action: " + type);
}

} // namespace

Config ConfigManager::load(const std::filesystem::path& file) const {
    std::ifstream input(file);
    if (!input) throw std::runtime_error("Cannot open config: " + file.string());

    Config config;
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;

        if (line.empty() || line[0] == '#') continue;

        const auto parts = split(line, '|');

        if (parts[0] == "WATCH") {
            if (parts.size() != 2) throw std::runtime_error("Invalid WATCH at line " + std::to_string(lineNumber));
            config.watchDirectory = parts[1];
            continue;
        }

        // RULE|name|EVENT|matcher_type|matcher_value|action_type|action_value
        if (parts[0] == "RULE" && parts.size() == 7) {
            config.rules.emplace_back(
                parts[1],
                parseEvent(parts[2]),
                makeMatcher(parts[3], parts[4]),
                makeAction(parts[5], parts[6])
            );
            continue;
        }

        throw std::runtime_error("Invalid config line " + std::to_string(lineNumber));
    }

    if (config.watchDirectory.empty())
        throw std::runtime_error("WATCH directory is required");

    return config;
}

} // namespace watchflow
