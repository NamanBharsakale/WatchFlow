#pragma once

#include "watchflow/core/FileEvent.hpp"

namespace watchflow {

class IMatcher {
public:
    virtual ~IMatcher() = default;
    virtual bool matches(const FileEvent& event) const = 0;
};

} // namespace watchflow
