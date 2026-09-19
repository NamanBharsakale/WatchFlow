#pragma once

#include "watchflow/core/FileEvent.hpp"
#include <functional>

namespace watchflow {

class IFileWatcher {
public:
    using EventCallback = std::function<void(const FileEvent&)>;

    virtual ~IFileWatcher() = default;
    virtual void start(EventCallback callback) = 0;
    virtual void stop() = 0;
};

} // namespace watchflow
