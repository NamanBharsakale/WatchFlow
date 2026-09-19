#pragma once

#include "watchflow/core/FileEvent.hpp"

namespace watchflow {

class ConsoleLogger {
public:
    void log(const FileEvent& event) const;
};

} // namespace watchflow
