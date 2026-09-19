#pragma once

#include <filesystem>

namespace watchflow {

enum class EventType {
    Created,
    Modified,
    Deleted,
    Moved
};

struct FileEvent {
    EventType type;
    std::filesystem::path path;
};

const char* toString(EventType type);

} // namespace watchflow
