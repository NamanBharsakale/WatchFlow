#include "watchflow/core/FileEvent.hpp"

namespace watchflow {

const char* toString(EventType type) {
    switch (type) {
        case EventType::Created: return "CREATED";
        case EventType::Modified: return "MODIFIED";
        case EventType::Deleted: return "DELETED";
        case EventType::Moved: return "MOVED";
    }
    return "UNKNOWN";
}

} // namespace watchflow
